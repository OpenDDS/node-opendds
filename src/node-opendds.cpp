#include "NodeDRListener.h"
#include "NodeQosConversion.h"

#include <node.h>
#include <v8.h>

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Registered_Data_Types.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/V8TypeConverter.h>

#include <ace/DLL_Manager.h>

#include <string>
#include <vector>
#include <cstring>

using namespace v8;
using OpenDDS::DCPS::Data_Types_Register;
using NodeOpenDDS::NodeDRListener;
using NodeOpenDDS::convertQos;

namespace {
  std::vector<DDS::DomainParticipant_var> participants_;
  std::string cft_name("CFT000001"); // unique names for ContentFilteredTopic

  void increment(std::string& name)
  {
    const size_t i = cft_name.find_first_not_of('0', 3);
    if (name[i] == '9') {
      if (i == 3) {
        name = "CFT000001";
      } else {
        name[i] = '0';
        name[i - 1] = '1';
      }
    } else {
      ++name[i];
    }
  }

  Handle<Value> create_participant(const Arguments& args);
  Handle<Value> delete_participant(const Arguments& args);
  Handle<Value> subscribe(const Arguments& args);
  Handle<Value> unsubscribe(const Arguments& args);

  Handle<Value> initialize(const Arguments& args)
  {
    HandleScope scope;
    std::vector<std::string> arg_storage;
    arg_storage.reserve(args.Length());
    ACE_ARGV_T<char> argv(false /*substitute env vars*/);
    for (int i = 0; i < args.Length(); ++i) {
      const Local<String> js_str = args[i]->ToString();
      arg_storage.push_back(std::string(js_str->Utf8Length(), '\0'));
      std::string& str = arg_storage.back();
      js_str->WriteUtf8(&str[0]);
      argv.add(str.c_str());
    }
    int argc = argv.argc();
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv.argv());
    const Handle<ObjectTemplate> ot = ObjectTemplate::New();
    ot->SetInternalFieldCount(1);
    node::SetMethod(ot, "create_participant", create_participant);
    node::SetMethod(ot, "delete_participant", delete_participant);
    const Local<Object> obj = ot->NewInstance();
    obj->SetPointerInInternalField(0, dpf._retn());
    return scope.Close(obj);
  }

  Handle<Value> create_participant(const Arguments& args)
  {
    HandleScope scope;
    const Local<Object> this_js = args.This();
    DDS::DomainId_t domain = 0;
    if (args.Length() > 0) {
      domain = static_cast<DDS::DomainId_t>(args[0]->NumberValue());
    }
    void* const internal = this_js->GetPointerFromInternalField(0);
    DDS::DomainParticipantFactory* const dpf =
      static_cast<DDS::DomainParticipantFactory*>(internal);
    DDS::DomainParticipantQos qos;
    dpf->get_default_participant_qos(qos);
    if (args.Length() > 1) {
      const Local<Object> qos_js = args[1]->ToObject();
      convertQos(qos, qos_js);
    }
    DDS::DomainParticipant_var dp = dpf->create_participant(domain, qos, 0, 0);
    if (!dp) {
      ThrowException(Exception::Error(String::New("couldn't create "
                                                  "DomainParticipant")));
      return scope.Close(Undefined());
    }
    participants_.push_back(dp);
    const Handle<ObjectTemplate> ot = ObjectTemplate::New();
    ot->SetInternalFieldCount(1);
    node::SetMethod(ot, "subscribe", subscribe);
    node::SetMethod(ot, "unsubscribe", unsubscribe);
    const Local<Object> obj = ot->NewInstance();
    obj->SetPointerInInternalField(0, dp._retn());
    return scope.Close(obj);
  }

  Handle<Value> delete_participant(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() == 0 || !args[0]->IsObject()) {
      ThrowException(Exception::TypeError(String::New("1 argument required")));
      return scope.Close(Undefined());
    }
    const Local<Object> js_obj = args[0]->ToObject();
    void* const internal = js_obj->GetPointerFromInternalField(0);
    const DDS::DomainParticipant_var part =
      static_cast<DDS::DomainParticipant*>(internal);
    js_obj->SetPointerInInternalField(0, 0);
    const std::vector<DDS::DomainParticipant_var>::iterator i =
      std::find(participants_.begin(), participants_.end(), part);
    if (i != participants_.end()) participants_.erase(i);
    part->delete_contained_entities();
    const DDS::DomainParticipantFactory_var dpf = TheParticipantFactory;
    dpf->delete_participant(part);
    return scope.Close(Undefined());
  }

  Handle<Value> subscribe(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() < 3) {
      ThrowException(Exception::TypeError(String::New("At least 3 "
                                                      "arguments required")));
      return scope.Close(Undefined());
    }
    if (!args[args.Length() - 1]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Last argument must "
                                                      "be a function")));
      return scope.Close(Undefined());
    }
    void* const internal = args.This()->GetPointerFromInternalField(0);
    DDS::DomainParticipant* const dp =
      static_cast<DDS::DomainParticipant*>(internal);

    const String::Utf8Value topic_name(args[0]);
    const String::Utf8Value topic_type(args[1]);
    OpenDDS::DCPS::TypeSupport* ts =
      Registered_Data_Types->lookup(dp, *topic_type);
    if (!ts) {
      ts = Registered_Data_Types->lookup(0, *topic_type);
      if (!ts) {
        ThrowException(Exception::Error(String::New("TypeSupport was not "
                                                    "registered")));
        return scope.Close(Undefined());
      }
      Registered_Data_Types->register_type(dp, *topic_type, ts);
    }
    const OpenDDS::DCPS::V8TypeConverter* const tc =
      dynamic_cast<const OpenDDS::DCPS::V8TypeConverter*>(ts);
    if (!tc) {
      ThrowException(Exception::Error(String::New("TypeSupport was not built "
                                                  "with support for V8.")));
      return scope.Close(Undefined());
    }

    DDS::Topic_var real_topic =
      dp->create_topic(*topic_name, *topic_type, TOPIC_QOS_DEFAULT, 0, 0);
    DDS::TopicDescription_var topic =
      DDS::TopicDescription::_duplicate(real_topic);
    if (!topic) {
      ThrowException(Exception::Error(String::New("couldn't create Topic")));
      return scope.Close(Undefined());
    }

    Local<Object> qos_js;
    if (args.Length() > 3 && args[2]->IsObject()) {
      qos_js = args[2]->ToObject();
    }
    const Handle<String> cft_str = String::NewSymbol("ContentFilteredTopic");
    if (*qos_js && qos_js->Has(cft_str)) {
      const Local<Object> cft_js = qos_js->Get(cft_str)->ToObject();
      const Handle<String> fe_str = String::NewSymbol("filter_expression"),
        ep_str = String::NewSymbol("expression_parameters");
      if (!cft_js->Has(fe_str)) {
        ThrowException(Exception::Error(String::New("filter_expression is "
                                                    "required in Content"
                                                    "FilteredTopic.")));
      }
      const String::Utf8Value filt(cft_js->Get(fe_str));
      DDS::StringSeq params;
      if (cft_js->Has(ep_str)) {
        const Local<Object> params_js = cft_js->Get(ep_str)->ToObject();
        const Local<Number> params_len =
          params_js->Get(String::NewSymbol("length"))->ToNumber();
        const uint32_t len = static_cast<uint32_t>(params_len->Value());
        params.length(len);
        for (uint32_t i = 0; i < len; ++i) {
          const String::Utf8Value pstr(params_js->Get(i));
          params[i] = *pstr;
        }
      }
      topic = dp->create_contentfilteredtopic(cft_name.c_str(), real_topic,
                                              *filt, params);
      increment(cft_name);
    }

    DDS::SubscriberQos sub_qos;
    dp->get_default_subscriber_qos(sub_qos);
    const Handle<String> subqos_str = String::NewSymbol("SubscriberQos");
    if (*qos_js && qos_js->Has(subqos_str)) {
      convertQos(sub_qos, qos_js->Get(subqos_str)->ToObject());
    }

    const DDS::Subscriber_var sub = dp->create_subscriber(sub_qos, 0, 0);
    if (!sub) {
      ThrowException(Exception::Error(String::New("couldn't create "
                                                  "Subscriber")));
      return scope.Close(Undefined());
    }

    Local<Value> cb = args[args.Length() - 1];
    NodeDRListener* const ndrl = new NodeDRListener(cb.As<Function>(), *tc);
    const DDS::DataReaderListener_var listen(ndrl);

    DDS::DataReaderQos dr_qos;
    sub->get_default_datareader_qos(dr_qos);
    const Handle<String> drqos_str = String::NewSymbol("DataReaderQos");
    if (*qos_js && qos_js->Has(drqos_str)) {
      convertQos(dr_qos, qos_js->Get(drqos_str)->ToObject());
    }

    DDS::DataReader_var dr = sub->create_datareader(topic, dr_qos, listen,
                                                    DDS::DATA_AVAILABLE_STATUS);
    if (!dr) {
      ThrowException(Exception::Error(String::New("couldn't create "
                                                  "DataReader")));
      return scope.Close(Undefined());
    }

    const Handle<ObjectTemplate> ot = ObjectTemplate::New();
    ot->SetInternalFieldCount(1);
    const Local<Object> obj = ot->NewInstance();
    obj->SetPointerInInternalField(0, dr._retn());
    ndrl->set_javascript_datareader(obj);
    return scope.Close(obj);
  }

  Handle<Value> unsubscribe(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsObject()) {
      ThrowException(Exception::TypeError(String::New("1 argument required")));
      return scope.Close(Undefined());
    }
    void* const internal = args.This()->GetPointerFromInternalField(0);
    DDS::DomainParticipant* const dp =
      static_cast<DDS::DomainParticipant*>(internal);

    const Local<Object> dr_js = args[0]->ToObject();
    void* const dr_obj = dr_js->GetPointerFromInternalField(0);
    DDS::DataReader_var dr = static_cast<DDS::DataReader*>(dr_obj);
    dr_js->SetPointerInInternalField(0, 0);

    const DDS::Subscriber_var sub = dr->get_subscriber();
    const DDS::TopicDescription_var td = dr->get_topicdescription();
    dr = 0;
    sub->delete_contained_entities();
    dp->delete_subscriber(sub);

    const DDS::ContentFilteredTopic_var cft =
      DDS::ContentFilteredTopic::_narrow(td);
    if (cft) {
      const DDS::Topic_var topic = cft->get_related_topic();
      dp->delete_contentfilteredtopic(cft);
      dp->delete_topic(topic);
    } else {
      const DDS::Topic_var topic = DDS::Topic::_narrow(td);
      dp->delete_topic(topic);
    }

    return scope.Close(Undefined());
  }

  Handle<Value> finalize(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() < 1) {
      ThrowException(Exception::TypeError(String::New("1 argument required")));
      return scope.Close(Undefined());
    }
    const Local<Object> dpf_js = args[0]->ToObject();
    void* const internal = dpf_js->GetPointerFromInternalField(0);
    const DDS::DomainParticipantFactory_var dpf =
      static_cast<DDS::DomainParticipantFactory*>(internal);
    dpf_js->SetPointerInInternalField(0, 0);

    for (size_t i = 0; i < participants_.size(); ++i) {
      DDS::DomainParticipant_var& part = participants_[i];
      part->delete_contained_entities();
      dpf->delete_participant(part);
    }
    participants_.clear();
    TheServiceParticipant->shutdown();
    return scope.Close(Undefined());
  }

  Handle<Value> load(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() == 0 || !args[0]->IsString()) {
      ThrowException(Exception::TypeError(String::New("1 argument required")));
      return scope.Close(Undefined());
    }
    const String::Utf8Value lib_js(args[0]);
    const ACE_TString lib(ACE_TEXT_CHAR_TO_TCHAR(*lib_js));
    const bool ok =
      ACE_DLL_Manager::instance()->open_dll(lib.c_str(),
                                            ACE_DEFAULT_SHLIB_MODE, 0);
    return scope.Close(Boolean::New(ok));
  }

  void init_node_opendds(Handle<Object> target)
  {
    node::SetMethod(target, "initialize", initialize);
    node::SetMethod(target, "finalize", finalize);
    node::SetMethod(target, "load", load);
  }
}

NODE_MODULE(node_opendds, init_node_opendds)
