#include "NodeQosConversion.h"

#include <stdexcept>
#include <cstring>

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
  if (qos_js->Has(prop_str)) {
    const Local<Object> props_policy_js = qos_js->Get(prop_str)->ToObject();
    const Local<String> name_str = Nan::New<String>("name").ToLocalChecked(),
      value_str = Nan::New<String>("value").ToLocalChecked();
    if (props_policy_js->Has(value_str)) {
      const Local<Object> props_array_js = props_policy_js->Get(value_str)->ToObject();
      const Local<String> len_str = Nan::New<String>("length").ToLocalChecked();

      // Iterate over the properties in the policy and add them to the native QoS
      const Nan::Maybe<uint32_t> props_array_len = Nan::To<uint32_t>(props_array_js->Get(len_str));
      const uint32_t len = props_array_len.FromMaybe(0);
      qos.property.value.length(len);
      for (uint32_t i = 0; i < len; ++i) {
        const Local<Object> property_js = props_array_js->Get(i)->ToObject();
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

void convertQos(DDS::SubscriberQos& qos, const Local<Object>& sqos_js)
{
  const Local<String> pres_str = Nan::New<String>("presentation").ToLocalChecked(),
    parti_str = Nan::New<String>("partition").ToLocalChecked(),
    grpd_str = Nan::New<String>("group_data").ToLocalChecked();

  if (sqos_js->Has(pres_str)) {
    const Local<Object> pres_js = sqos_js->Get(pres_str)->ToObject();
    const Local<String> scope_str = Nan::New<String>("access_scope").ToLocalChecked(),
      coher_str = Nan::New<String>("coherent_access").ToLocalChecked(),
      order_str = Nan::New<String>("ordered_access").ToLocalChecked();
    if (pres_js->Has(scope_str)) {
      const Nan::Utf8String scope(pres_js->Get(scope_str));
      if (0 == std::strcmp(*scope, "INSTANCE_PRESENTATION_QOS")) {
        qos.presentation.access_scope = DDS::INSTANCE_PRESENTATION_QOS;
      } else if (0 == std::strcmp(*scope, "TOPIC_PRESENTATION_QOS")) {
        qos.presentation.access_scope = DDS::TOPIC_PRESENTATION_QOS;
      } else if (0 == std::strcmp(*scope, "GROUP_PRESENTATION_QOS")) {
        qos.presentation.access_scope = DDS::GROUP_PRESENTATION_QOS;
      }
    }
    if (pres_js->Has(coher_str)) {
      qos.presentation.coherent_access =
        pres_js->Get(coher_str)->BooleanValue();
    }
    if (pres_js->Has(order_str)) {
      qos.presentation.ordered_access = pres_js->Get(order_str)->BooleanValue();
    }
  }

  if (sqos_js->Has(parti_str)) {
    const Local<Object> parti_js = sqos_js->Get(parti_str)->ToObject();
    const Local<String> len_str = Nan::New<String>("length").ToLocalChecked();
    const Nan::Maybe<uint32_t> parti_len =
      Nan::To<uint32_t>(parti_js->Get(len_str));
    const uint32_t len = parti_len.FromMaybe(0);
    qos.partition.name.length(len);
    for (uint32_t i = 0; i < len; ++i) {
      const Nan::Utf8String elt(parti_js->Get(i));
      qos.partition.name[i] = *elt;
    }
  }

  if (sqos_js->Has(grpd_str)) {
    convert(qos.group_data.value, sqos_js->Get(grpd_str)->ToString());
  }
}

void convert(DDS::Duration_t& dur, const Local<Object>& dur_js)
{
  const Local<String> sec_str = Nan::New<String>("sec").ToLocalChecked(),
    nanosec_str = Nan::New<String>("nanosec").ToLocalChecked();
  if (dur_js->Has(sec_str)) {
    dur.sec = dur_js->Get(sec_str)->Int32Value();
  }
  if (dur_js->Has(nanosec_str)) {
    dur.nanosec = dur_js->Get(nanosec_str)->Uint32Value();
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
    rdlife_str = Nan::New<String>("reader_data_lifecycle").ToLocalChecked(),
    kind_str = Nan::New<String>("kind").ToLocalChecked();

  if (drqos_js->Has(durab_str)) {
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

  if (drqos_js->Has(deadln_str)) {
    convert(qos.deadline.period, drqos_js->Get(deadln_str)->ToObject());
  }

  if (drqos_js->Has(latbud_str)) {
    convert(qos.latency_budget.duration, drqos_js->Get(latbud_str)->ToObject());
  }

  if (drqos_js->Has(liveli_str)) {
    const Local<Object> liveliness = drqos_js->Get(liveli_str)->ToObject();
    if (liveliness->Has(kind_str)) {
      const Nan::Utf8String lv(liveliness->Get(kind_str));
      if (0 == std::strcmp(*lv, "AUTOMATIC_LIVELINESS_QOS")) {
        qos.liveliness.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
      } else if (0 == std::strcmp(*lv,
                                  "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")) {
        qos.liveliness.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
      } else if (0 == std::strcmp(*lv, "MANUAL_BY_TOPIC_LIVELINESS_QOS")) {
        qos.liveliness.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
      }
    }
    const Local<String> lease_duration_str =
      Nan::New<String>("lease_duration").ToLocalChecked();
    if (liveliness->Has(lease_duration_str)) {
      convert(qos.liveliness.lease_duration,
              liveliness->Get(lease_duration_str)->ToObject());
    }
  }

  if (drqos_js->Has(reliab_str)) {
    const Local<Object> reliability = drqos_js->Get(reliab_str)->ToObject();
    if (reliability->Has(kind_str)) {
      const Nan::Utf8String rl(reliability->Get(kind_str));
      if (0 == std::strcmp(*rl, "BEST_EFFORT_RELIABILITY_QOS")) {
        qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
      } else if (0 == std::strcmp(*rl, "RELIABLE_RELIABILITY_QOS")) {
        qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      }
    }
    const Local<String> mbt_str = Nan::New<String>("max_blocking_time").ToLocalChecked();
    if (reliability->Has(mbt_str)) {
      convert(qos.reliability.max_blocking_time,
              reliability->Get(mbt_str)->ToObject());
    }
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

  if (drqos_js->Has(history_str)) {
    const Local<Object> history = drqos_js->Get(history_str)->ToObject();
    if (history->Has(kind_str)) {
      const Nan::Utf8String hk(history->Get(kind_str));
      if (0 == std::strcmp(*hk, "KEEP_LAST_HISTORY_QOS")) {
        qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
      } else if (0 == std::strcmp(*hk, "KEEP_ALL_HISTORY_QOS")) {
        qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
      }
    }
    const Local<String> depth_str = Nan::New<String>("depth").ToLocalChecked();
    if (history->Has(depth_str)) {
      qos.history.depth = history->Get(depth_str)->Int32Value();
    }
  }

  if (drqos_js->Has(reslim_str)) {
    const Local<Object> reslim = drqos_js->Get(reslim_str)->ToObject();
    const Local<String> ms_str = Nan::New<String>("max_samples").ToLocalChecked(),
      mi_str = Nan::New<String>("max_instances").ToLocalChecked(),
      mspi_str = Nan::New<String>("max_samples_per_instance").ToLocalChecked();
    if (reslim->Has(ms_str)) {
      qos.resource_limits.max_samples = reslim->Get(ms_str)->Int32Value();
    }
    if (reslim->Has(mi_str)) {
      qos.resource_limits.max_instances = reslim->Get(mi_str)->Int32Value();
    }
    if (reslim->Has(mspi_str)) {
      qos.resource_limits.max_samples_per_instance =
        reslim->Get(mspi_str)->Int32Value();
    }
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

  if (drqos_js->Has(tbfilter_str)) {
    convert(qos.time_based_filter.minimum_separation,
            drqos_js->Get(tbfilter_str)->ToObject());
  }

  if (drqos_js->Has(rdlife_str)) {
    const Local<Object> rdlife = drqos_js->Get(rdlife_str)->ToObject();
    const Local<String> ansd_str =
      Nan::New<String>("autopurge_nowriter_samples_delay").ToLocalChecked(),
      adsd_str = Nan::New<String>("autopurge_disposed_samples_delay").ToLocalChecked();
    if (rdlife->Has(ansd_str)) {
      convert(qos.reader_data_lifecycle.autopurge_nowriter_samples_delay,
              rdlife->Get(ansd_str)->ToObject());
    }
    if (rdlife->Has(adsd_str)) {
      convert(qos.reader_data_lifecycle.autopurge_disposed_samples_delay,
              rdlife->Get(adsd_str)->ToObject());
    }
  }
}


}
