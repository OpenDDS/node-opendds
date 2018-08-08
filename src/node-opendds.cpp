#include "NodeDRListener.h"
#include "NodeQosConversion.h"

#include <nan.h>

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Registered_Data_Types.h>
#include <dds/DCPS/Marked_Default_Qos.h>
#include <dds/DCPS/V8TypeConverter.h>

#include <ace/DLL_Manager.h>
#include <ace/Init_ACE.h>

#include <string>
#include <vector>
#include <cstring>

using namespace v8;
using OpenDDS::DCPS::Data_Types_Register;
using NodeOpenDDS::NodeDRListener;
using NodeOpenDDS::convertQos;

#define V8STR(str) Nan::New<String>((str)).ToLocalChecked()
#define RUN(str) \
  Nan::RunScript(Nan::CompileScript(V8STR((str))).ToLocalChecked())

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

  void create_participant(const Nan::FunctionCallbackInfo<Value>& fci);
  void delete_participant(const Nan::FunctionCallbackInfo<Value>& fci);
  void subscribe(const Nan::FunctionCallbackInfo<Value>& fci);
  void unsubscribe(const Nan::FunctionCallbackInfo<Value>& fci);

  void initialize(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    ACE::init();
    std::vector<std::string> arg_storage;
    arg_storage.reserve(fci.Length());
    ACE_ARGV_T<char> argv(false /*substitute env vars*/);
    for (int i = 0; i < fci.Length(); ++i) {
      const Local<String> js_str = fci[i]->ToString();
      arg_storage.push_back(std::string(js_str->Utf8Length(), '\0'));
      std::string& str = arg_storage.back();
      js_str->WriteUtf8(&str[0]);
      argv.add(str.c_str());
    }
    int argc = argv.argc();
    DDS::DomainParticipantFactory_var dpf =
      TheParticipantFactoryWithArgs(argc, argv.argv());
    const Local<ObjectTemplate> ot = Nan::New<ObjectTemplate>();
    ot->SetInternalFieldCount(1);
    Nan::SetMethod(ot, "create_participant", create_participant);
    Nan::SetMethod(ot, "delete_participant", delete_participant);
    const Local<Object> obj = ot->NewInstance();
    Nan::SetInternalFieldPointer(obj, 0, dpf._retn());
    fci.GetReturnValue().Set(obj);
  }

  void create_participant(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    const Local<Object> this_js = fci.This();
    DDS::DomainId_t domain = 0;
    if (fci.Length() > 0) {
      domain = static_cast<DDS::DomainId_t>(fci[0]->NumberValue());
    }
    void* const internal = Nan::GetInternalFieldPointer(this_js, 0);
    DDS::DomainParticipantFactory* const dpf =
      static_cast<DDS::DomainParticipantFactory*>(internal);
    DDS::DomainParticipantQos qos;
    dpf->get_default_participant_qos(qos);
    if (fci.Length() > 1) {
      const Local<Object> qos_js = fci[1]->ToObject();
      convertQos(qos, qos_js);
    }
    DDS::DomainParticipant_var dp = dpf->create_participant(domain, qos, 0, 0);
    if (!dp) {
      Nan::ThrowError("couldn't create DomainParticipant");
      fci.GetReturnValue().SetUndefined();
      return;
    }
    participants_.push_back(dp);
    const Local<ObjectTemplate> ot = Nan::New<ObjectTemplate>();
    ot->SetInternalFieldCount(1);
    Nan::SetMethod(ot, "subscribe", subscribe);
    Nan::SetMethod(ot, "unsubscribe", unsubscribe);
    const Local<Object> obj = ot->NewInstance();
    Nan::SetInternalFieldPointer(obj, 0, dp._retn());
    fci.GetReturnValue().Set(obj);
  }

  void delete_participant(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    if (fci.Length() == 0 || !fci[0]->IsObject()) {
      Nan::ThrowTypeError("1 argument required");
      fci.GetReturnValue().SetUndefined();
      return;
    }
    const Local<Object> js_obj = fci[0]->ToObject();
    void* const internal = Nan::GetInternalFieldPointer(js_obj, 0);
    const DDS::DomainParticipant_var part =
      static_cast<DDS::DomainParticipant*>(internal);
    Nan::SetInternalFieldPointer(js_obj, 0, 0);
    const std::vector<DDS::DomainParticipant_var>::iterator i =
      std::find(participants_.begin(), participants_.end(), part);
    if (i != participants_.end()) participants_.erase(i);
    part->delete_contained_entities();
    const DDS::DomainParticipantFactory_var dpf = TheParticipantFactory;
    dpf->delete_participant(part);
    fci.GetReturnValue().SetUndefined();
  }

  void subscribe(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    if (fci.Length() < 3) {
      Nan::ThrowTypeError("At least 3 arguments required");
      fci.GetReturnValue().SetUndefined();
      return;
    }
    if (!fci[fci.Length() - 1]->IsFunction()) {
      Nan::ThrowTypeError("Last argument must be a function");
      fci.GetReturnValue().SetUndefined();
      return;
    }
    void* const internal = Nan::GetInternalFieldPointer(fci.This(), 0);
    DDS::DomainParticipant* const dp =
      static_cast<DDS::DomainParticipant*>(internal);

    const String::Utf8Value topic_name(fci[0]);
    const String::Utf8Value topic_type(fci[1]);
    OpenDDS::DCPS::TypeSupport* ts =
      Registered_Data_Types->lookup(dp, *topic_type);
    if (!ts) {
      ts = Registered_Data_Types->lookup(0, *topic_type);
      if (!ts) {
        Nan::ThrowError("TypeSupport was not registered");
        fci.GetReturnValue().SetUndefined();
        return;
      }
      Registered_Data_Types->register_type(dp, *topic_type, ts);
    }
    const OpenDDS::DCPS::V8TypeConverter* const tc =
      dynamic_cast<const OpenDDS::DCPS::V8TypeConverter*>(ts);
    if (!tc) {
      Nan::ThrowError("TypeSupport was not built with support for V8.");
      fci.GetReturnValue().SetUndefined();
      return;
    }

    DDS::Topic_var real_topic =
      dp->create_topic(*topic_name, *topic_type, TOPIC_QOS_DEFAULT, 0, 0);
    DDS::TopicDescription_var topic =
      DDS::TopicDescription::_duplicate(real_topic);
    if (!topic) {
      Nan::ThrowError("couldn't create Topic");
      fci.GetReturnValue().SetUndefined();
      return;
    }

    Local<Object> qos_js;
    if (fci.Length() > 3 && fci[2]->IsObject()) {
      qos_js = fci[2]->ToObject();
    }
    Nan::MaybeLocal<String> cft_str = Nan::New<String>("ContentFilteredTopic");
    const Local<String> cft_lstr = cft_str.ToLocalChecked();
    if (*qos_js && qos_js->Has(cft_lstr)) {
      const Local<Object> cft_js = qos_js->Get(cft_lstr)->ToObject();
      Nan::MaybeLocal<String> fe_str = Nan::New<String>("filter_expression"),
        ep_str = Nan::New<String>("expression_parameters");
      const Local<String> fe_lstr = fe_str.ToLocalChecked();
      if (!cft_js->Has(fe_lstr)) {
        Nan::ThrowError("filter_expression is required in "
                        "ContentFilteredTopic.");
        fci.GetReturnValue().SetUndefined();
        return;
      }
      const String::Utf8Value filt(cft_js->Get(fe_lstr));
      DDS::StringSeq params;
      const Local<String> ep_lstr = ep_str.ToLocalChecked();
      if (cft_js->Has(ep_lstr)) {
        const Local<Object> params_js = cft_js->Get(ep_lstr)->ToObject();
        const Nan::Maybe<uint32_t> len =
          Nan::To<uint32_t>(params_js->Get(Nan::New<String>("length")
                                           .ToLocalChecked()));
        params.length(len.FromMaybe(0));
        for (uint32_t i = 0; i < params.length(); ++i) {
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
    Nan::MaybeLocal<String> subqos_str = Nan::New<String>("SubscriberQos");
    const Local<String> subqos_lstr = subqos_str.ToLocalChecked();
    if (*qos_js && qos_js->Has(subqos_lstr)) {
      convertQos(sub_qos, qos_js->Get(subqos_lstr)->ToObject());
    }

    const DDS::Subscriber_var sub = dp->create_subscriber(sub_qos, 0, 0);
    if (!sub) {
      Nan::ThrowError("couldn't create Subscriber");
      fci.GetReturnValue().SetUndefined();
      return;
    }

    Local<Value> cb = fci[fci.Length() - 1];
    NodeDRListener* const ndrl = new NodeDRListener(dp, cb.As<Function>(), *tc);
    const DDS::DataReaderListener_var listen(ndrl);

    DDS::DataReaderQos dr_qos;
    sub->get_default_datareader_qos(dr_qos);
    Nan::MaybeLocal<String> drqos_str = Nan::New<String>("DataReaderQos");
    const Local<String> drqos_lstr = drqos_str.ToLocalChecked();
    if (*qos_js && qos_js->Has(drqos_lstr)) {
      convertQos(dr_qos, qos_js->Get(drqos_lstr)->ToObject());
    }

    DDS::DataReader_var dr = sub->create_datareader(topic, dr_qos, listen,
                                                    DDS::DATA_AVAILABLE_STATUS);
    if (!dr) {
      Nan::ThrowError("couldn't create DataReader");
      fci.GetReturnValue().SetUndefined();
      return;
    }

    const Local<ObjectTemplate> ot = Nan::New<ObjectTemplate>();
    ot->SetInternalFieldCount(1);
    const Local<Object> obj = ot->NewInstance();
    Nan::SetInternalFieldPointer(obj, 0, dr._retn());
    ndrl->set_javascript_datareader(obj);
    fci.GetReturnValue().Set(obj);
  }

  void unsubscribe(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    if (fci.Length() < 1 || !fci[0]->IsObject()) {
      Nan::ThrowTypeError("1 argument required");
      fci.GetReturnValue().SetUndefined();
      return;
    }

    // Get the NodeDRListener
    const Local<Object> dr_js = fci[0]->ToObject();
    void* const dr_obj = Nan::GetInternalFieldPointer(dr_js, 0);
    DDS::DataReader* dr = static_cast<DDS::DataReader*>(dr_obj);
    NodeDRListener* const ndrl = dynamic_cast<NodeDRListener*>(dr->get_listener());

    ndrl->unsubscribe();

    fci.GetReturnValue().SetUndefined();
  }

  void finalize(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    if (fci.Length() < 1) {
      Nan::ThrowTypeError("1 argument required");
      fci.GetReturnValue().SetUndefined();
      return;
    }
    const Local<Object> dpf_js = fci[0]->ToObject();
    void* const internal = Nan::GetInternalFieldPointer(dpf_js, 0);
    const DDS::DomainParticipantFactory_var dpf =
      static_cast<DDS::DomainParticipantFactory*>(internal);
    Nan::SetInternalFieldPointer(dpf_js, 0, 0);

    for (size_t i = 0; i < participants_.size(); ++i) {
      DDS::DomainParticipant_var& part = participants_[i];
      part->delete_contained_entities();
      dpf->delete_participant(part);
    }
    participants_.clear();
    TheServiceParticipant->shutdown();
    ACE::fini();
    fci.GetReturnValue().SetUndefined();
  }

  void load(const Nan::FunctionCallbackInfo<Value>& fci)
  {
    if (fci.Length() == 0 || !fci[0]->IsString()) {
      Nan::ThrowTypeError("1 argument required");
      fci.GetReturnValue().SetUndefined();
      return;
    }
    const String::Utf8Value lib_js(fci[0]);
    const ACE_TString lib(ACE_TEXT_CHAR_TO_TCHAR(*lib_js));
    const bool ok =
      ACE_DLL_Manager::instance()->open_dll(lib.c_str(),
                                            ACE_DEFAULT_SHLIB_MODE, 0);
    fci.GetReturnValue().Set(ok);
  }

  void init_node_opendds(Local<Object> target)
  {
    Nan::SetMethod(target, "initialize", initialize);
    Nan::SetMethod(target, "finalize", finalize);
    Nan::SetMethod(target, "load", load);
  }
}

NODE_MODULE(node_opendds, init_node_opendds)
