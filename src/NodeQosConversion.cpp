#include "NodeQosConversion.h"

#include <stdexcept>
#include <cstring>
#include <iostream>

using namespace v8;

namespace NodeOpenDDS {

void convert(DDS::OctetSeq& seq, const Local<String>& js_str)
{
  seq.length(js_str->Utf8Length());
  js_str->WriteUtf8(reinterpret_cast<char*>(seq.get_buffer()), seq.length());
}

void convertQos(DDS::DomainParticipantQos& qos,
                const Local<Object>& qos_js)
{
  const Local<String> user_data = Nan::New<String>("user_data").ToLocalChecked();
  if (qos_js->Has(user_data)) {
    convert(qos.user_data.value, qos_js->Get(user_data)->ToString());
  }
  // entity_factory QoS is not supported since there is no enable() method

  // If it exists, get the PropertyQosPolicy that enables security features:
  const Local<String> prop_str = Nan::New<String>("property").ToLocalChecked();
  if (qos_js->Has(prop_str) && qos_js->Get(prop_str)->IsObject()) {
    const Local<Object> props_policy_js = qos_js->Get(prop_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> name_str = Nan::New<String>("name").ToLocalChecked(),
      value_str = Nan::New<String>("value").ToLocalChecked();
    if (props_policy_js->Has(value_str) && props_policy_js->Get(value_str)->IsObject()) {
      const Local<Object> props_array_js = props_policy_js->Get(value_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
      const Local<String> len_str = Nan::New<String>("length").ToLocalChecked();

      // Iterate over the properties in the policy and add them to the native QoS
      const Nan::Maybe<uint32_t> props_array_len = Nan::To<uint32_t>(props_array_js->Get(len_str));
      const uint32_t len = props_array_len.FromMaybe(0);
      qos.property.value.length(len);
      for (uint32_t i = 0; i < len; ++i) {
        if (props_array_js->Get(i)->IsObject()) {
          const Local<Object> property_js = props_array_js->Get(i)->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
          if (property_js->Has(name_str) && property_js->Has(value_str)) {
            const Nan::Utf8String name(property_js->Get(name_str));
            qos.property.value[i].name = *name;
            const Nan::Utf8String value(property_js->Get(value_str));
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

  if (qos_js->Has(scope_str)) {
    const Nan::Utf8String scope(qos_js->Get(scope_str));
    if (0 == std::strcmp(*scope, "INSTANCE_PRESENTATION_QOS")) {
      qos.access_scope = DDS::INSTANCE_PRESENTATION_QOS;
    } else if (0 == std::strcmp(*scope, "TOPIC_PRESENTATION_QOS")) {
      qos.access_scope = DDS::TOPIC_PRESENTATION_QOS;
    } else if (0 == std::strcmp(*scope, "GROUP_PRESENTATION_QOS")) {
      qos.access_scope = DDS::GROUP_PRESENTATION_QOS;
    }
  }
  if (qos_js->Has(coher_str)) {
    qos.coherent_access = qos_js->Get(coher_str)->BooleanValue();
  }
  if (qos_js->Has(order_str)) {
    qos.ordered_access = qos_js->Get(order_str)->BooleanValue();
  }
}

void convertQos(DDS::PartitionQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> len_str = Nan::New<String>("length").ToLocalChecked();
  const Nan::Maybe<uint32_t> parti_len = Nan::To<uint32_t>(qos_js->Get(len_str));
  const uint32_t len = parti_len.FromMaybe(0);
  qos.name.length(len);
  for (uint32_t i = 0; i < len; ++i) {
    const Nan::Utf8String elt(qos_js->Get(i));
    qos.name[i] = *elt;
  }
}

void convertQos(DDS::PublisherQos& qos, const Local<Object>& qos_js)
{
  const Local<String> pres_str = Nan::New<String>("presentation").ToLocalChecked(),
    parti_str = Nan::New<String>("partition").ToLocalChecked(),
    grpd_str = Nan::New<String>("group_data").ToLocalChecked();

  if (qos_js->Has(pres_str) && qos_js->Get(pres_str)->IsObject()) {
    convertQos(qos.presentation, qos_js->Get(pres_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (qos_js->Has(parti_str) && qos_js->Get(parti_str)->IsObject()) {
    convertQos(qos.partition, qos_js->Get(parti_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (qos_js->Has(grpd_str)) {
    convert(qos.group_data.value, qos_js->Get(grpd_str)->ToString());
  }
}

void convertQos(DDS::SubscriberQos& qos, const Local<Object>& qos_js)
{
  const Local<String> pres_str = Nan::New<String>("presentation").ToLocalChecked(),
    parti_str = Nan::New<String>("partition").ToLocalChecked(),
    grpd_str = Nan::New<String>("group_data").ToLocalChecked();

  if (qos_js->Has(pres_str) && qos_js->Get(pres_str)->IsObject()) {
    convertQos(qos.presentation, qos_js->Get(pres_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (qos_js->Has(parti_str) && qos_js->Get(parti_str)->IsObject()) {
    convertQos(qos.partition, qos_js->Get(parti_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (qos_js->Has(grpd_str)) {
    convert(qos.group_data.value, qos_js->Get(grpd_str)->ToString());
  }
}

void convert(DDS::Duration_t& dur, const Local<Object>& dur_js)
{
  const Local<String> sec_str = Nan::New<String>("sec").ToLocalChecked(),
    nanosec_str = Nan::New<String>("nanosec").ToLocalChecked();

  if (dur_js->Has(sec_str) && dur_js->Get(sec_str)->IsInt32()) {
    dur.sec = dur_js->Get(sec_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
  if (dur_js->Has(nanosec_str) && dur_js->Get(nanosec_str)->IsUint32()) {
    dur.nanosec = dur_js->Get(nanosec_str)->Uint32Value(Nan::GetCurrentContext()).ToChecked();
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

  if (qos_js->Has(scd_str) && qos_js->Get(scd_str)->IsObject()) {
    convert(qos.service_cleanup_delay, qos_js->Get(scd_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }
  if (qos_js->Has(hk_str)) {
    const Nan::Utf8String dur(qos_js->Get(hk_str));
    if (0 == std::strcmp(*dur, "KEEP_LAST_HISTORY_QOS")) {
      qos.history_kind = DDS::KEEP_LAST_HISTORY_QOS;
    } else if (0 == std::strcmp(*dur, "KEEP_ALL_HISTORY_QOS")) {
      qos.history_kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
  }
  if (qos_js->Has(hd_str) && qos_js->Get(hd_str)->IsInt32()) {
    qos.history_depth = qos_js->Get(hd_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
  if (qos_js->Has(ms_str) && qos_js->Get(ms_str)->IsInt32()) {
    qos.max_samples = qos_js->Get(ms_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
  if (qos_js->Has(mi_str) && qos_js->Get(mi_str)->IsInt32()) {
    qos.max_instances = qos_js->Get(mi_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
  if (qos_js->Has(mspi_str) && qos_js->Get(mspi_str)->IsInt32()) {
    qos.max_samples_per_instance = qos_js->Get(mspi_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
}

void convertQos(DDS::LivelinessQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> kind_str = Nan::New<String>("kind").ToLocalChecked(),
    lease_duration_str = Nan::New<String>("lease_duration").ToLocalChecked();

  if (qos_js->Has(kind_str)) {
    const Nan::Utf8String lv(qos_js->Get(kind_str));
    if (0 == std::strcmp(*lv, "AUTOMATIC_LIVELINESS_QOS")) {
      qos.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
    } else if (0 == std::strcmp(*lv, "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")) {
      qos.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
    } else if (0 == std::strcmp(*lv, "MANUAL_BY_TOPIC_LIVELINESS_QOS")) {
      qos.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
    }
  }
  if (qos_js->Has(lease_duration_str) && qos_js->Get(lease_duration_str)->IsObject()) {
    convert(qos.lease_duration, qos_js->Get(lease_duration_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }
}

void convertQos(DDS::ReliabilityQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> kind_str = Nan::New<String>("kind").ToLocalChecked(),
    mbt_str = Nan::New<String>("max_blocking_time").ToLocalChecked();

  if (qos_js->Has(kind_str)) {
    const Nan::Utf8String rl(qos_js->Get(kind_str));
    if (0 == std::strcmp(*rl, "BEST_EFFORT_RELIABILITY_QOS")) {
      qos.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
    } else if (0 == std::strcmp(*rl, "RELIABLE_RELIABILITY_QOS")) {
      qos.kind = DDS::RELIABLE_RELIABILITY_QOS;
    }
  }
  if (qos_js->Has(mbt_str) && qos_js->Get(mbt_str)->IsObject()) {
    convert(qos.max_blocking_time, qos_js->Get(mbt_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }
}

void convertQos(DDS::HistoryQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> kind_str = Nan::New<String>("kind").ToLocalChecked(),
    depth_str = Nan::New<String>("depth").ToLocalChecked();

  if (qos_js->Has(kind_str)) {
    const Nan::Utf8String hk(qos_js->Get(kind_str));
    if (0 == std::strcmp(*hk, "KEEP_LAST_HISTORY_QOS")) {
      qos.kind = DDS::KEEP_LAST_HISTORY_QOS;
    } else if (0 == std::strcmp(*hk, "KEEP_ALL_HISTORY_QOS")) {
      qos.kind = DDS::KEEP_ALL_HISTORY_QOS;
    }
  }
  if (qos_js->Has(depth_str) && qos_js->Get(depth_str)->IsInt32()) {
    qos.depth = qos_js->Get(depth_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
}

void convertQos(DDS::ResourceLimitsQosPolicy& qos, const Local<Object>& qos_js)
{
  const Local<String> ms_str = Nan::New<String>("max_samples").ToLocalChecked(),
    mi_str = Nan::New<String>("max_instances").ToLocalChecked(),
    mspi_str = Nan::New<String>("max_samples_per_instance").ToLocalChecked();

  if (qos_js->Has(ms_str) && qos_js->Get(ms_str)->IsInt32()) {
    qos.max_samples = qos_js->Get(ms_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
  if (qos_js->Has(mi_str) && qos_js->Get(mi_str)->IsInt32()) {
    qos.max_instances = qos_js->Get(mi_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }
  if (qos_js->Has(mspi_str) && qos_js->Get(mspi_str)->IsInt32()) {
    qos.max_samples_per_instance =
      qos_js->Get(mspi_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
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

  if (dwqos_js->Has(durab_str)) {
    const Nan::Utf8String dur(dwqos_js->Get(durab_str));
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

  if (dwqos_js->Has(durabsvc_str) && dwqos_js->Get(durabsvc_str)->IsObject()) {
    convertQos(qos.durability_service, dwqos_js->Get(durabsvc_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(deadln_str) && dwqos_js->Get(deadln_str)->IsObject()) {
    convert(qos.deadline.period, dwqos_js->Get(deadln_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(latbud_str) && dwqos_js->Get(latbud_str)->IsObject()) {
    convert(qos.latency_budget.duration, dwqos_js->Get(latbud_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(liveli_str) && dwqos_js->Get(liveli_str)->IsObject()) {
    convertQos(qos.liveliness, dwqos_js->Get(liveli_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(reliab_str) && dwqos_js->Get(reliab_str)->IsObject()) {
    convertQos(qos.reliability, dwqos_js->Get(reliab_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(destord_str)) {
    const Nan::Utf8String ord(dwqos_js->Get(destord_str));
    if (0 == std::strcmp(*ord, "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    } else if (0 == std::strcmp(*ord,
                                "BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    }
  }

  if (dwqos_js->Has(history_str) && dwqos_js->Get(history_str)->IsObject()) {
    convertQos(qos.history, dwqos_js->Get(history_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(reslim_str) && dwqos_js->Get(reslim_str)->IsObject()) {
    convertQos(qos.resource_limits, dwqos_js->Get(reslim_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(tranpri_str) && dwqos_js->Get(tranpri_str)->IsInt32()) {
    qos.transport_priority.value = dwqos_js->Get(tranpri_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
  }

  if (dwqos_js->Has(lifespan_str) && dwqos_js->Get(lifespan_str)->IsObject()) {
    convert(qos.lifespan.duration, dwqos_js->Get(lifespan_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(userdata_str) && dwqos_js->Get(userdata_str)->IsString()) {
    convert(qos.user_data.value, dwqos_js->Get(userdata_str)->ToString(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (dwqos_js->Has(ownership_str) && dwqos_js->Get(ownership_str)->IsString()) {
    const Nan::Utf8String own(dwqos_js->Get(ownership_str));
    if (0 == std::strcmp(*own, "SHARED_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::SHARED_OWNERSHIP_QOS;
    } else if (0 == std::strcmp(*own, "EXCLUSIVE_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
    }
  }

  if (dwqos_js->Has(ostrength_str) && dwqos_js->Get(ostrength_str)->IsObject()) {
    const Local<Object> ostrength = dwqos_js->Get(ostrength_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> v_str = Nan::New<String>("value").ToLocalChecked();
    if (ostrength->Has(v_str) && ostrength->Get(v_str)->IsInt32()) {
      qos.ownership_strength.value = ostrength->Get(v_str)->Int32Value(Nan::GetCurrentContext()).ToChecked();
    }
  }

  if (dwqos_js->Has(wdlife_str) && dwqos_js->Get(wdlife_str)->IsObject()) {
    const Local<Object> wdlife = dwqos_js->Get(wdlife_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> adui_str = Nan::New<String>("autodispose_unregistered_instances").ToLocalChecked();
    if (wdlife->Has(adui_str) && wdlife->Get(adui_str)->IsBoolean()) {
      qos.writer_data_lifecycle.autodispose_unregistered_instances = wdlife->Get(adui_str)->BooleanValue(Nan::GetCurrentContext()).ToChecked();
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

  if (drqos_js->Has(durab_str) && drqos_js->Get(durab_str)->IsString()) {
    const Nan::Utf8String dur(drqos_js->Get(durab_str));
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

  if (drqos_js->Has(deadln_str) && drqos_js->Get(deadln_str)->IsObject()) {
    convert(qos.deadline.period, drqos_js->Get(deadln_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(latbud_str) && drqos_js->Get(latbud_str)->IsObject()) {
    convert(qos.latency_budget.duration, drqos_js->Get(latbud_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(liveli_str) && drqos_js->Get(liveli_str)->IsObject()) {
    convertQos(qos.liveliness, drqos_js->Get(liveli_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(reliab_str) && drqos_js->Get(reliab_str)->IsObject()) {
    convertQos(qos.reliability, drqos_js->Get(reliab_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(destord_str)) {
    const Nan::Utf8String ord(drqos_js->Get(destord_str));
    if (0 == std::strcmp(*ord, "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS;
    } else if (0 == std::strcmp(*ord,
                                "BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS")) {
      qos.destination_order.kind =
        DDS::BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS;
    }
  }

  if (drqos_js->Has(history_str) && drqos_js->Get(history_str)->IsObject()) {
    convertQos(qos.history, drqos_js->Get(history_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(reslim_str) && drqos_js->Get(reslim_str)->IsObject()) {
    convertQos(qos.resource_limits, drqos_js->Get(reslim_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(userdata_str)) {
    convert(qos.user_data.value, drqos_js->Get(userdata_str)->ToString());
  }

  if (drqos_js->Has(ownership_str)) {
    const Nan::Utf8String own(drqos_js->Get(ownership_str));
    if (0 == std::strcmp(*own, "SHARED_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::SHARED_OWNERSHIP_QOS;
    } else if (0 == std::strcmp(*own, "EXCLUSIVE_OWNERSHIP_QOS")) {
      qos.ownership.kind = DDS::EXCLUSIVE_OWNERSHIP_QOS;
    }
  }

  if (drqos_js->Has(tbfilter_str) && drqos_js->Get(tbfilter_str)->IsObject()) {
    convert(qos.time_based_filter.minimum_separation,
            drqos_js->Get(tbfilter_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
  }

  if (drqos_js->Has(rdlife_str) && drqos_js->Get(rdlife_str)->IsObject()) {
    const Local<Object> rdlife = drqos_js->Get(rdlife_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked();
    const Local<String> ansd_str =
      Nan::New<String>("autopurge_nowriter_samples_delay").ToLocalChecked(),
      adsd_str = Nan::New<String>("autopurge_disposed_samples_delay").ToLocalChecked();
    if (rdlife->Has(ansd_str) && rdlife->Get(ansd_str)->IsObject()) {
      convert(qos.reader_data_lifecycle.autopurge_nowriter_samples_delay,
              rdlife->Get(ansd_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
    }
    if (rdlife->Has(adsd_str) && rdlife->Get(adsd_str)->IsObject()) {
      convert(qos.reader_data_lifecycle.autopurge_disposed_samples_delay,
              rdlife->Get(adsd_str)->ToObject(Nan::GetCurrentContext()).ToLocalChecked());
    }
  }
}


}
