#include "NodeQosConversion.h"

#include <v8.h>

#include <cstring>

using namespace v8;

namespace NodeOpenDDS {

void convert(DDS::OctetSeq& seq, const Local<String>& js_str)
{
  seq.length(js_str->Utf8Length());
  js_str->WriteUtf8(reinterpret_cast<char*>(seq.get_buffer()));
}

void convertQos(DDS::DomainParticipantQos& qos,
                const Local<Object>& qos_js)
{
  const Handle<String> user_data = String::NewSymbol("user_data");
  if (qos_js->Has(user_data)) {
    convert(qos.user_data.value, qos_js->Get(user_data)->ToString());
  }
  // entity_factory QoS is not supported since there is no enable() method
}

void convertQos(DDS::SubscriberQos& qos, const Local<Object>& sqos_js)
{
  const Handle<String> pres_str = String::NewSymbol("presentation"),
    parti_str = String::NewSymbol("partition"),
    grpd_str = String::NewSymbol("group_data");

  if (sqos_js->Has(pres_str)) {
    const Local<Object> pres_js = sqos_js->Get(pres_str)->ToObject();
    const Handle<String> scope_str = String::NewSymbol("access_scope"),
      coher_str = String::NewSymbol("coherent_access"),
      order_str = String::NewSymbol("ordered_access");
    if (pres_js->Has(scope_str)) {
      const String::Utf8Value scope(pres_js->Get(scope_str));
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
    const Local<Number> parti_len =
      parti_js->Get(String::NewSymbol("length"))->ToNumber();
    const uint32_t len = static_cast<uint32_t>(parti_len->Value());
    qos.partition.name.length(len);
    for (uint32_t i = 0; i < len; ++i) {
      const String::Utf8Value elt(parti_js->Get(i));
      qos.partition.name[i] = *elt;
    }
  }

  if (sqos_js->Has(grpd_str)) {
    convert(qos.group_data.value, sqos_js->Get(grpd_str)->ToString());
  }
}

void convert(DDS::Duration_t& dur, const Local<Object>& dur_js)
{
  const Handle<String> sec_str = String::NewSymbol("sec"),
    nanosec_str = String::NewSymbol("nanosec");
  if (dur_js->Has(sec_str)) {
    dur.sec = dur_js->Get(sec_str)->Int32Value();
  }
  if (dur_js->Has(nanosec_str)) {
    dur.nanosec = dur_js->Get(nanosec_str)->Uint32Value();
  }
}

void convertQos(DDS::DataReaderQos& qos, const Local<Object>& drqos_js)
{
  const Handle<String> durab_str = String::NewSymbol("durability"),
    deadln_str = String::NewSymbol("deadline"),
    latbud_str = String::NewSymbol("latency_budget"),
    liveli_str = String::NewSymbol("liveliness"),
    reliab_str = String::NewSymbol("reliability"),
    destord_str = String::NewSymbol("destination_order"),
    history_str = String::NewSymbol("history"),
    reslim_str = String::NewSymbol("resource_limits"),
    userdata_str = String::NewSymbol("user_data"),
    ownership_str = String::NewSymbol("ownership"),
    tbfilter_str = String::NewSymbol("time_based_filter"),
    rdlife_str = String::NewSymbol("reader_data_lifecycle"),
    kind_str = String::NewSymbol("kind");

  if (drqos_js->Has(durab_str)) {
    const String::Utf8Value dur(drqos_js->Get(durab_str));
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
      const String::Utf8Value lv(liveliness->Get(kind_str));
      if (0 == std::strcmp(*lv, "AUTOMATIC_LIVELINESS_QOS")) {
        qos.liveliness.kind = DDS::AUTOMATIC_LIVELINESS_QOS;
      } else if (0 == std::strcmp(*lv,
                                  "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS")) {
        qos.liveliness.kind = DDS::MANUAL_BY_PARTICIPANT_LIVELINESS_QOS;
      } else if (0 == std::strcmp(*lv, "MANUAL_BY_TOPIC_LIVELINESS_QOS")) {
        qos.liveliness.kind = DDS::MANUAL_BY_TOPIC_LIVELINESS_QOS;
      }   
    }
    const Handle<String> lease_duration_str =
      String::NewSymbol("lease_duration");
    if (liveliness->Has(lease_duration_str)) {
      convert(qos.liveliness.lease_duration,
              liveliness->Get(lease_duration_str)->ToObject());
    }
  }

  if (drqos_js->Has(reliab_str)) {
    const Local<Object> reliability = drqos_js->Get(reliab_str)->ToObject();
    if (reliability->Has(kind_str)) {
      const String::Utf8Value rl(reliability->Get(kind_str));
      if (0 == std::strcmp(*rl, "BEST_EFFORT_RELIABILITY_QOS")) {
        qos.reliability.kind = DDS::BEST_EFFORT_RELIABILITY_QOS;
      } else if (0 == std::strcmp(*rl, "RELIABLE_RELIABILITY_QOS")) {
        qos.reliability.kind = DDS::RELIABLE_RELIABILITY_QOS;
      }
    }
    const Handle<String> mbt_str = String::NewSymbol("max_blocking_time");
    if (reliability->Has(mbt_str)) {
      convert(qos.reliability.max_blocking_time,
              reliability->Get(mbt_str)->ToObject());
    }
  }

  if (drqos_js->Has(destord_str)) {
    const String::Utf8Value ord(drqos_js->Get(destord_str));
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
      const String::Utf8Value hk(history->Get(kind_str));
      if (0 == std::strcmp(*hk, "KEEP_LAST_HISTORY_QOS")) {
        qos.history.kind = DDS::KEEP_LAST_HISTORY_QOS;
      } else if (0 == std::strcmp(*hk, "KEEP_ALL_HISTORY_QOS")) {
        qos.history.kind = DDS::KEEP_ALL_HISTORY_QOS;
      }
    }
    const Handle<String> depth_str = String::NewSymbol("depth");
    if (history->Has(depth_str)) {
      qos.history.depth = history->Get(depth_str)->Int32Value();
    }
  }

  if (drqos_js->Has(reslim_str)) {
    const Local<Object> reslim = drqos_js->Get(reslim_str)->ToObject();
    const Handle<String> ms_str = String::NewSymbol("max_samples"),
      mi_str = String::NewSymbol("max_instances"),
      mspi_str = String::NewSymbol("max_samples_per_instance");
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
    const String::Utf8Value own(drqos_js->Get(ownership_str));
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
    const Handle<String> ansd_str =
      String::NewSymbol("autopurge_nowriter_samples_delay"),
      adsd_str = String::NewSymbol("autopurge_disposed_samples_delay");
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

