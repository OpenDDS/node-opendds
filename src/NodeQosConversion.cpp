#include "NodeQosConversion.h"

#include <stdexcept>
#include <cstring>
#include <iostream>

using namespace v8;

namespace NodeOpenDDS {

void convert(DDS::OctetSeq& seq, const Local<String>& js_str)
{
  const Nan::Utf8String value(js_str);
  seq.length(value.length());
  memcpy(seq.get_buffer(), *value, value.length());
}

void convertQos(DDS::DomainParticipantQos& qos,
                const Local<Object>& qos_js)
{
  const Local<String> user_data = Nan::New<String>("user_data").ToLocalChecked();
  if (Nan::Has(qos_js, user_data).ToChecked()) {
    convert(qos.user_data.value, Nan::Get(qos_js, user_data).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked());
  }
  // entity_factory QoS is not supported since there is no enable() method

  // If it exists, get the PropertyQosPolicy that enables security features:
  const Local<String> prop_str = Nan::New<String>("property").ToLocalChecked();
  if (Nan::Has(qos_js, prop_str).ToChecked() && Nan::Get(qos_js, prop_str).ToLocalChecked()->IsObject()) {
    const Local<Object> props_policy_js = Nan::Get(qos_js, prop_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> name_str = Nan::New<String>("name").ToLocalChecked(),
      value_str = Nan::New<String>("value").ToLocalChecked();
    if (Nan::Has(props_policy_js, value_str).ToChecked() && Nan::Get(props_policy_js, value_str).ToLocalChecked()->IsObject()) {
      const Local<Object> props_array_js = Nan::Get(props_policy_js, value_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
      const Local<String> len_str = Nan::New<String>("length").ToLocalChecked();

      // Iterate over the properties in the policy and add them to the native QoS
      const Nan::Maybe<uint32_t> props_array_len = Nan::To<uint32_t>(Nan::Get(props_array_js, len_str).ToLocalChecked());
      const uint32_t len = props_array_len.FromMaybe(0);
      qos.property.value.length(len);
      for (uint32_t i = 0; i < len; ++i) {
        if (Nan::Get(props_array_js, i).ToLocalChecked()->IsObject()) {
          const Local<Object> property_js = Nan::Get(props_array_js, i).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
          if (Nan::Has(property_js, name_str).ToChecked() && Nan::Has(property_js, value_str).ToChecked()) {
            const Nan::Utf8String name(Nan::Get(property_js, name_str).ToLocalChecked());
            qos.property.value[i].name = *name;
            const Nan::Utf8String value(Nan::Get(property_js, value_str).ToLocalChecked());
            qos.property.value[i].value = *value;
          } else {
            throw std::runtime_error(
              "All properties must be objects with name and value attributes");
          }
        }
      }
    }
  }
}

void convertQos(DDS::PresentationQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> scope_str = Nan::New<String>("access_scope").ToLocalChecked(),
    coher_str = Nan::New<String>("coherent_access").ToLocalChecked(),
    order_str = Nan::New<String>("ordered_access").ToLocalChecked();

  if (Nan::Has(qos_js, scope_str).ToChecked()) {
    const Nan::Utf8String scope(Nan::Get(qos_js, scope_str).ToLocalChecked());
    if (0 == std::strcmp(*scope, "INSTANCE_PRESENTATION_QOS")) {
      qos.access_scope = DDS::INSTANCE_PRESENTATION_QOS;
    } else if (0 == std::strcmp(*scope, "TOPIC_PRESENTATION_QOS")) {
      qos.access_scope = DDS::TOPIC_PRESENTATION_QOS;
    } else if (0 == std::strcmp(*scope, "GROUP_PRESENTATION_QOS")) {
      qos.access_scope = DDS::GROUP_PRESENTATION_QOS;
    }
  }
  if (Nan::Has(qos_js, coher_str).ToChecked()) {
    qos.coherent_access = Nan::To<bool>(Nan::Get(qos_js, coher_str).ToLocalChecked()).FromMaybe(false);
  }
  if (Nan::Has(qos_js, order_str).ToChecked()) {
    qos.ordered_access = Nan::To<bool>(Nan::Get(qos_js, order_str).ToLocalChecked()).FromMaybe(false);
  }
}

void convertQos(DDS::PartitionQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> len_str = Nan::New<String>("length").ToLocalChecked();
  const Nan::Maybe<uint32_t> parti_len = Nan::To<uint32_t>(Nan::Get(qos_js, len_str).ToLocalChecked());
  const uint32_t len = parti_len.FromMaybe(0);
  qos.name.length(len);
  for (uint32_t i = 0; i < len; ++i) {
    const Nan::Utf8String elt(Nan::Get(qos_js, i).ToLocalChecked());
    qos.name[i] = *elt;
  }
}

void convertQos(DDS::PublisherQos& qos, const Local<Object>& qos_js)
{
  const Local<String> pres_str = Nan::New<String>("presentation").ToLocalChecked(),
    parti_str = Nan::New<String>("partition").ToLocalChecked(),
    grpd_str = Nan::New<String>("group_data").ToLocalChecked();

  if (Nan::Has(qos_js, pres_str).ToChecked() && Nan::Get(qos_js, pres_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.presentation, Nan::Get(qos_js, pres_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(qos_js, parti_str).ToChecked() && Nan::Get(qos_js, parti_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.partition, Nan::Get(qos_js, parti_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(qos_js, grpd_str).ToChecked()) {
    convert(qos.group_data.value, Nan::Get(qos_js, grpd_str).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked());
  }
}

void convertQos(DDS::SubscriberQos& qos, const Local<Object>& qos_js)
{
  const Local<String> pres_str = Nan::New<String>("presentation").ToLocalChecked(),
    parti_str = Nan::New<String>("partition").ToLocalChecked(),
    grpd_str = Nan::New<String>("group_data").ToLocalChecked();

  if (Nan::Has(qos_js, pres_str).ToChecked() && Nan::Get(qos_js, pres_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.presentation, Nan::Get(qos_js, pres_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(qos_js, parti_str).ToChecked() && Nan::Get(qos_js, parti_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.partition, Nan::Get(qos_js, parti_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(qos_js, grpd_str).ToChecked()) {
    convert(qos.group_data.value, Nan::Get(qos_js, grpd_str).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked());
  }
}

void convert(DDS::Duration_t& dur, const Local<Object>& dur_js)
{
  const Local<String> sec_str = Nan::New<String>("sec").ToLocalChecked(),
    nanosec_str = Nan::New<String>("nanosec").ToLocalChecked();

  if (Nan::Has(dur_js, sec_str).ToChecked()) {
    dur.sec = Nan::To<int32_t>(Nan::Get(dur_js, sec_str).ToLocalChecked()).FromMaybe(0);
  }
  if (Nan::Has(dur_js, nanosec_str).ToChecked()) {
    dur.nanosec = Nan::To<uint32_t>(Nan::Get(dur_js, nanosec_str).ToLocalChecked()).FromMaybe(0);
  }
}

void convertQos(DDS::DurabilityServiceQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> scd_str = Nan::New<String>("service_cleanup_delay").ToLocalChecked(),
    hk_str = Nan::New<String>("history_kind").ToLocalChecked(),
    hd_str = Nan::New<String>("history_depth").ToLocalChecked(),
    ms_str = Nan::New<String>("max_samples").ToLocalChecked(),
    mi_str = Nan::New<String>("max_instances").ToLocalChecked(),
    mspi_str = Nan::New<String>("max_samples_per_instance").ToLocalChecked();

  if (Nan::Has(qos_js, scd_str).ToChecked() && Nan::Get(qos_js, scd_str).ToLocalChecked()->IsObject()) {
    convert(qos.service_cleanup_delay, Nan::Get(qos_js, scd_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }
  if (Nan::Has(qos_js, hk_str).ToChecked()) {
    const Nan::Utf8String dur(Nan::Get(qos_js, hk_str).ToLocalChecked());
    if (0 == std::strcmp(*dur, "KEEP_LAST_HISTORY_QOS")) {
      qos.history_kind = DDS::KEEP_LAST_HISTORY_QOS;
    } else if (0 == std::strcmp(*dur, "KEEP_ALL_HISTORY_QOS")) {
      qos.history_kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
  }
  if (Nan::Has(qos_js, hd_str).ToChecked()) {
    qos.history_depth = Nan::To<int32_t>(Nan::Get(qos_js, hd_str).ToLocalChecked()).FromMaybe(0);
  }
  if (Nan::Has(qos_js, ms_str).ToChecked()) {
    qos.max_samples = Nan::To<int32_t>(Nan::Get(qos_js, ms_str).ToLocalChecked()).FromMaybe(0);
  }
  if (Nan::Has(qos_js, mi_str).ToChecked()) {
    qos.max_instances = Nan::To<int32_t>(Nan::Get(qos_js, mi_str).ToLocalChecked()).FromMaybe(0);
  }
  if (Nan::Has(qos_js, mspi_str).ToChecked()) {
    qos.max_samples_per_instance = Nan::To<int32_t>(Nan::Get(qos_js, mspi_str).ToLocalChecked()).FromMaybe(0);
  }
}

void convertQos(DDS::LivelinessQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> kind_str = Nan::New<String>("kind").ToLocalChecked(),
    lease_duration_str = Nan::New<String>("lease_duration").ToLocalChecked();

  if (Nan::Has(qos_js, kind_str).ToChecked()) {
    const Nan::Utf8String lv(Nan::Get(qos_js, kind_str).ToLocalChecked());
    if (0 == std::strcmp(*lv, "AUTOMATIC_LIVELINESS_QOS")) {
      qos.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
    } else if (0 == std::strcmp(*lv, "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")) {
      qos.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    } else if (0 == std::strcmp(*lv, "MANUAL_BY_TOPIC_LIVELINESS_QOS")) {
      qos.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
    }
  }
  if (Nan::Has(qos_js, lease_duration_str).ToChecked() && Nan::Get(qos_js, lease_duration_str).ToLocalChecked()->IsObject()) {
    convert(qos.lease_duration, Nan::Get(qos_js, lease_duration_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }
}

void convertQos(DDS::ReliabilityQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> kind_str = Nan::New<String>("kind").ToLocalChecked(),
    mbt_str = Nan::New<String>("max_blocking_time").ToLocalChecked();

  if (Nan::Has(qos_js, kind_str).ToChecked()) {
    const Nan::Utf8String rl(Nan::Get(qos_js, kind_str).ToLocalChecked());
    if (0 == std::strcmp(*rl, "BEST_EFFORT_RELIABILITY_QOS")) {
      qos.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
    } else if (0 == std::strcmp(*rl, "RELIABLE_RELIABILITY_QOS")) {
      qos.kind = DDS::RELIABLE_RELIABILITY_QOS;
    }
  }
  if (Nan::Has(qos_js, mbt_str).ToChecked() && Nan::Get(qos_js, mbt_str).ToLocalChecked()->IsObject()) {
    convert(qos.max_blocking_time, Nan::Get(qos_js, mbt_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }
}

void convertQos(DDS::HistoryQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> kind_str = Nan::New<String>("kind").ToLocalChecked(),
    depth_str = Nan::New<String>("depth").ToLocalChecked();

  if (Nan::Has(qos_js, kind_str).ToChecked()) {
    const Nan::Utf8String hk(Nan::Get(qos_js, kind_str).ToLocalChecked());
    if (0 == std::strcmp(*hk, "KEEP_LAST_HISTORY_QOS")) {
      qos.kind = DDS::KEEP_LAST_HISTORY_QOS;
    } else if (0 == std::strcmp(*hk, "KEEP_ALL_HISTORY_QOS")) {
      qos.kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
  }
  if (Nan::Has(qos_js, depth_str).ToChecked()) {
    qos.depth = Nan::To<int32_t>(Nan::Get(qos_js, depth_str).ToLocalChecked()).FromMaybe(0);
  }
}

void convertQos(DDS::ResourceLimitsQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> ms_str = Nan::New<String>("max_samples").ToLocalChecked(),
    mi_str = Nan::New<String>("max_instances").ToLocalChecked(),
    mspi_str = Nan::New<String>("max_samples_per_instance").ToLocalChecked();

  if (Nan::Has(qos_js, ms_str).ToChecked()) {
    qos.max_samples = Nan::To<int32_t>(Nan::Get(qos_js, ms_str).ToLocalChecked()).FromMaybe(0);
  }
  if (Nan::Has(qos_js, mi_str).ToChecked()) {
    qos.max_instances = Nan::To<int32_t>(Nan::Get(qos_js, mi_str).ToLocalChecked()).FromMaybe(0);
  }
  if (Nan::Has(qos_js, mspi_str).ToChecked()) {
    qos.max_samples_per_instance = Nan::To<int32_t>(Nan::Get(qos_js, mspi_str).ToLocalChecked()).FromMaybe(0);
  }
}

void convertQos(DDS::DataWriterQos& qos, const Local<Object>& dwqos_js)
{
  const Local<String> durab_str = Nan::New<String>("durability").ToLocalChecked(),
    durabsvc_str = Nan::New<String>("durability_service").ToLocalChecked(),
    deadln_str = Nan::New<String>("deadline").ToLocalChecked(),
    latbud_str = Nan::New<String>("latency_budget").ToLocalChecked(),
    liveli_str = Nan::New<String>("liveliness").ToLocalChecked(),
    reliab_str = Nan::New<String>("reliability").ToLocalChecked(),
    destord_str = Nan::New<String>("destination_order").ToLocalChecked(),
    history_str = Nan::New<String>("history").ToLocalChecked(),
    reslim_str = Nan::New<String>("resource_limits").ToLocalChecked(),
    tranpri_str = Nan::New<String>("transport_priorty").ToLocalChecked(),
    lifespan_str = Nan::New<String>("lifespan").ToLocalChecked(),
    userdata_str = Nan::New<String>("user_data").ToLocalChecked(),
    ownership_str = Nan::New<String>("ownership").ToLocalChecked(),
    ostrength_str = Nan::New<String>("ownership_strength").ToLocalChecked(),
    wdlife_str = Nan::New<String>("writer_data_lifecycle").ToLocalChecked();

  if (Nan::Has(dwqos_js, durab_str).ToChecked()) {
    const Nan::Utf8String dur(Nan::Get(dwqos_js, durab_str).ToLocalChecked());
    if (0 == std::strcmp(*dur, "VOLATILE_DURABILITY_QOS")) {
      qos.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
    } else if (0 == std::strcmp(*dur, "TRANSIENT_LOCAL_DURABILITY_QOS")) {
      qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
    } else if (0 == std::strcmp(*dur, "TRANSIENT_DURABILITY_QOS")) {
      qos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
    } else if (0 == std::strcmp(*dur, "PERSISTENT_DURABILITY_QOS")) {
      qos.durability.kind = DDS::PERSISTENT_DURABILITY_QOS;
    }
  }

  if (Nan::Has(dwqos_js, durabsvc_str).ToChecked() && Nan::Get(dwqos_js, durabsvc_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.durability_service, Nan::Get(dwqos_js, durabsvc_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, deadln_str).ToChecked() && Nan::Get(dwqos_js, deadln_str).ToLocalChecked()->IsObject()) {
    convert(qos.deadline.period, Nan::Get(dwqos_js, deadln_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, latbud_str).ToChecked() && Nan::Get(dwqos_js, latbud_str).ToLocalChecked()->IsObject()) {
    convert(qos.latency_budget.duration, Nan::Get(dwqos_js, latbud_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, liveli_str).ToChecked() && Nan::Get(dwqos_js, liveli_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.liveliness, Nan::Get(dwqos_js, liveli_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, reliab_str).ToChecked() && Nan::Get(dwqos_js, reliab_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.reliability, Nan::Get(dwqos_js, reliab_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, destord_str).ToChecked()) {
    const Nan::Utf8String ord(Nan::Get(dwqos_js, destord_str).ToLocalChecked());
    if (0 == std::strcmp(*ord, "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    } else if (0 == std::strcmp(*ord,
                                "BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    }
  }

  if (Nan::Has(dwqos_js, history_str).ToChecked() && Nan::Get(dwqos_js, history_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.history, Nan::Get(dwqos_js, history_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, reslim_str).ToChecked() && Nan::Get(dwqos_js, reslim_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.resource_limits, Nan::Get(dwqos_js, reslim_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, tranpri_str).ToChecked()) {
    qos.transport_priority.value = Nan::To<int32_t>(Nan::Get(dwqos_js, tranpri_str).ToLocalChecked()).FromMaybe(0);
  }

  if (Nan::Has(dwqos_js, lifespan_str).ToChecked() && Nan::Get(dwqos_js, lifespan_str).ToLocalChecked()->IsObject()) {
    convert(qos.lifespan.duration, Nan::Get(dwqos_js, lifespan_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, userdata_str).ToChecked() && Nan::Get(dwqos_js, userdata_str).ToLocalChecked()->IsString()) {
    convert(qos.user_data.value, Nan::Get(dwqos_js, userdata_str).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(dwqos_js, ownership_str).ToChecked() && Nan::Get(dwqos_js, ownership_str).ToLocalChecked()->IsString()) {
    const Nan::Utf8String own(Nan::Get(dwqos_js, ownership_str).ToLocalChecked());
    if (0 == std::strcmp(*own, "SHARED_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::SHARED_OWNERSHIP_QOS;
    } else if (0 == std::strcmp(*own, "EXCLUSIVE_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
    }
  }

  if (Nan::Has(dwqos_js, ostrength_str).ToChecked() && Nan::Get(dwqos_js, ostrength_str).ToLocalChecked()->IsObject()) {
    const Local<Object> ostrength = Nan::Get(dwqos_js, ostrength_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> v_str = Nan::New<String>("value").ToLocalChecked();
    if (Nan::Has(ostrength, v_str).ToChecked()) {
      qos.ownership_strength.value = Nan::To<int32_t>(Nan::Get(ostrength, v_str).ToLocalChecked()).FromMaybe(0);
    }
  }

  if (Nan::Has(dwqos_js, wdlife_str).ToChecked() && Nan::Get(dwqos_js, wdlife_str).ToLocalChecked()->IsObject()) {
    const Local<Object> wdlife = Nan::Get(dwqos_js, wdlife_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> adui_str = Nan::New<String>("autodispose_unregistered_instances").ToLocalChecked();
    if (Nan::Has(wdlife, adui_str).ToChecked()) {
      qos.writer_data_lifecycle.autodispose_unregistered_instances = Nan::To<bool>(Nan::Get(wdlife, adui_str).ToLocalChecked()).FromMaybe(false);
    }
  }
}

void convertQos(DDS::DataReaderQos& qos, const Local<Object>& drqos_js)
{
  const Local<String> durab_str = Nan::New<String>("durability").ToLocalChecked(),
    deadln_str = Nan::New<String>("deadline").ToLocalChecked(),
    latbud_str = Nan::New<String>("latency_budget").ToLocalChecked(),
    liveli_str = Nan::New<String>("liveliness").ToLocalChecked(),
    reliab_str = Nan::New<String>("reliability").ToLocalChecked(),
    destord_str = Nan::New<String>("destination_order").ToLocalChecked(),
    history_str = Nan::New<String>("history").ToLocalChecked(),
    reslim_str = Nan::New<String>("resource_limits").ToLocalChecked(),
    userdata_str = Nan::New<String>("user_data").ToLocalChecked(),
    ownership_str = Nan::New<String>("ownership").ToLocalChecked(),
    tbfilter_str = Nan::New<String>("time_based_filter").ToLocalChecked(),
    rdlife_str = Nan::New<String>("reader_data_lifecycle").ToLocalChecked();

  if (Nan::Has(drqos_js, durab_str).ToChecked() && Nan::Get(drqos_js, durab_str).ToLocalChecked()->IsString()) {
    const Nan::Utf8String dur(Nan::Get(drqos_js, durab_str).ToLocalChecked());
    if (0 == std::strcmp(*dur, "VOLATILE_DURABILITY_QOS")) {
      qos.durability.kind = DDS::VOLATILE_DURABILITY_QOS;
    } else if (0 == std::strcmp(*dur, "TRANSIENT_LOCAL_DURABILITY_QOS")) {
      qos.durability.kind = DDS::TRANSIENT_LOCAL_DURABILITY_QOS;
    } else if (0 == std::strcmp(*dur, "TRANSIENT_DURABILITY_QOS")) {
      qos.durability.kind = DDS::TRANSIENT_DURABILITY_QOS;
    } else if (0 == std::strcmp(*dur, "PERSISTENT_DURABILITY_QOS")) {
      qos.durability.kind = DDS::PERSISTENT_DURABILITY_QOS;
    }
  }

  if (Nan::Has(drqos_js, deadln_str).ToChecked() && Nan::Get(drqos_js, deadln_str).ToLocalChecked()->IsObject()) {
    convert(qos.deadline.period, Nan::Get(drqos_js, deadln_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, latbud_str).ToChecked() && Nan::Get(drqos_js, latbud_str).ToLocalChecked()->IsObject()) {
    convert(qos.latency_budget.duration, Nan::Get(drqos_js, latbud_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, liveli_str).ToChecked() && Nan::Get(drqos_js, liveli_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.liveliness, Nan::Get(drqos_js, liveli_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, reliab_str).ToChecked() && Nan::Get(drqos_js, reliab_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.reliability, Nan::Get(drqos_js, reliab_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, destord_str).ToChecked()) {
    const Nan::Utf8String ord(Nan::Get(drqos_js, destord_str).ToLocalChecked());
    if (0 == std::strcmp(*ord, "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    } else if (0 == std::strcmp(*ord,
                                "BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    }
  }

  if (Nan::Has(drqos_js, history_str).ToChecked() && Nan::Get(drqos_js, history_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.history, Nan::Get(drqos_js, history_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, reslim_str).ToChecked() && Nan::Get(drqos_js, reslim_str).ToLocalChecked()->IsObject()) {
    convertQos(qos.resource_limits, Nan::Get(drqos_js, reslim_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, userdata_str).ToChecked()) {
    convert(qos.user_data.value, Nan::Get(drqos_js, userdata_str).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, ownership_str).ToChecked()) {
    const Nan::Utf8String own(Nan::Get(drqos_js, ownership_str).ToLocalChecked());
    if (0 == std::strcmp(*own, "SHARED_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::SHARED_OWNERSHIP_QOS;
    } else if (0 == std::strcmp(*own, "EXCLUSIVE_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
    }
  }

  if (Nan::Has(drqos_js, tbfilter_str).ToChecked() && Nan::Get(drqos_js, tbfilter_str).ToLocalChecked()->IsObject()) {
    convert(qos.time_based_filter.minimum_separation,
            Nan::Get(drqos_js, tbfilter_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (Nan::Has(drqos_js, rdlife_str).ToChecked() && Nan::Get(drqos_js, rdlife_str).ToLocalChecked()->IsObject()) {
    const Local<Object> rdlife = Nan::Get(drqos_js, rdlife_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> ansd_str =
      Nan::New<String>("autopurge_nowriter_samples_delay").ToLocalChecked(),
      adsd_str = Nan::New<String>("autopurge_disposed_samples_delay").ToLocalChecked();
    if (Nan::Has(rdlife, ansd_str).ToChecked() && Nan::Get(rdlife, ansd_str).ToLocalChecked()->IsObject()) {
      convert(qos.reader_data_lifecycle.autopurge_nowriter_samples_delay,
              Nan::Get(rdlife, ansd_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
    }
    if (Nan::Has(rdlife, adsd_str).ToChecked() && Nan::Get(rdlife, adsd_str).ToLocalChecked()->IsObject()) {
      convert(qos.reader_data_lifecycle.autopurge_disposed_samples_delay,
              Nan::Get(rdlife, adsd_str).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
    }
  }
}


}
