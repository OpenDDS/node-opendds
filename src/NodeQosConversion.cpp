#include "NodeQosConversion.h"

#include <v8.h>

#include <cstring>

using namespace v8;

namespace NodeOpenDDS {

void convertQos(DDS::DomainParticipantQos& qos,
                const v8::Local<v8::Object>& qos_js)
{
  const Handle<String> user_data = String::NewSymbol("user_data");
  if (qos_js->Has(user_data)) {
    const Local<String> ud = qos_js->Get(user_data)->ToString();
    qos.user_data.value.length(ud->Utf8Length());
    CORBA::Octet* const buffer = qos.user_data.value.get_buffer();
    ud->WriteUtf8(reinterpret_cast<char*>(buffer));
  }
  // entity_factory QoS is not supported since there is no enable() method
}

void convertQos(DDS::SubscriberQos& qos,
                const v8::Local<v8::Object>& sqos_js)
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
    const Local<String> gd = sqos_js->Get(grpd_str)->ToString();
    qos.group_data.value.length(gd->Utf8Length());
    CORBA::Octet* const buffer = qos.group_data.value.get_buffer();
    gd->WriteUtf8(reinterpret_cast<char*>(buffer));
  }
}

void convertQos(DDS::DataReaderQos& qos,
                const v8::Local<v8::Object>& sqos_js)
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
    rdlife_str = String::NewSymbol("reader_data_lifecycle");
  //TODO
}


}

