#include <node.h>
#include <v8.h>

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Registered_Data_Types.h>

#include <string>
#include <vector>

using namespace v8;
using OpenDDS::DCPS::Data_Types_Register;

namespace {
  std::vector<DDS::DomainParticipant_var> participants_;

  class NodeDRListener
    : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener> {
  public:
    explicit NodeDRListener(const Local<Function>& callback)
      : callback_(Persistent<Function>::New(callback))
      , async_uv_(this)
    {
      uv_async_init(uv_default_loop(), &async_uv_, async_cb);
    }

    static void async_cb(uv_async_t* async_uv, int /*status*/)
    {
      static_cast<AsyncUv*>(async_uv)->outer_->async();
    }

    ~NodeDRListener()
    {
      uv_close((uv_handle_t*)&async_uv_, 0);
      callback_.Dispose();
      js_dr_.Dispose();
    }

    typedef DDS::RequestedDeadlineMissedStatus RDMStatus;
    void on_requested_deadline_missed(DDS::DataReader*, const RDMStatus&) {}
    typedef DDS::RequestedIncompatibleQosStatus RIQStatus;
    void on_requested_incompatible_qos(DDS::DataReader*, const RIQStatus&) {}
    void on_sample_rejected(DDS::DataReader*,
                            const DDS::SampleRejectedStatus&) {}
    void on_liveliness_changed(DDS::DataReader*,
                               const DDS::LivelinessChangedStatus&) {}
    void on_subscription_matched(DDS::DataReader*,
                                 const DDS::SubscriptionMatchedStatus&) {}
    void on_sample_lost(DDS::DataReader*, const DDS::SampleLostStatus&) {}

    void on_data_available(DDS::DataReader*)
    {
      uv_async_send(&async_uv_);
    }

    void async() // called from libuv event loop
    {
      // invoke callback with (datareader, sampleinfo, sample)
      Handle<Value> argv[] = {js_dr_,
                              v8::Object::New(),
                              v8::Object::New() };
      node::MakeCallback(Context::GetCurrent()->Global(), callback_,
                         sizeof(argv) / sizeof(argv[0]), argv);
    }

    void set_javascript_datareader(const Local<v8::Object>& js_dr)
    {
      js_dr_ = Persistent<v8::Object>::New(js_dr);
    }

  private:
    NodeDRListener(const NodeDRListener&);
    NodeDRListener& operator=(const NodeDRListener&);
    Persistent<Function> callback_;
    Persistent<v8::Object> js_dr_;

    struct AsyncUv : uv_async_t {
      explicit AsyncUv(NodeDRListener* outer) : outer_(outer) {}
      NodeDRListener* outer_;
    } async_uv_;
  };

  Handle<Value> create_participant(const Arguments& args);
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
    DDS::DomainParticipantFactory* dpf =
      static_cast<DDS::DomainParticipantFactory*>(internal);
    DDS::DomainParticipantQos qos;
    dpf->get_default_participant_qos(qos);
    if (args.Length() > 1) {
      const Local<Object> qos_js = args[1]->ToObject();
      const Handle<String> user_data = String::NewSymbol("user_data");
      if (qos_js->Has(user_data)) {
        const Local<String> ud = qos_js->Get(user_data)->ToString();
        qos.user_data.value.length(ud->Utf8Length());
        CORBA::Octet* const buffer = qos.user_data.value.get_buffer();
        ud->WriteUtf8(reinterpret_cast<char*>(buffer));
      }
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

  // participant.subscribe(topic_name, topic_type, {filter/qos}, callback)
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
    DDS::DomainParticipant* dp =
      static_cast<DDS::DomainParticipant*>(internal);

    const String::Utf8Value topic_name(args[0]);
    const String::Utf8Value topic_type(args[1]);
    OpenDDS::DCPS::TypeSupport* ts =
      Registered_Data_Types->lookup(dp, *topic_type);
    if (!ts) {
      ThrowException(Exception::Error(String::New("TypeSupport was not "
                                                  "registered")));
      return scope.Close(Undefined());
    }

    DDS::TopicQos topic_qos;
    dp->get_default_topic_qos(topic_qos); //TODO: user's topic QoS
    DDS::Topic_var topic =
      dp->create_topic(*topic_name, *topic_type, topic_qos, 0, 0);
    if (!topic) {
      ThrowException(Exception::Error(String::New("couldn't create Topic")));
      return scope.Close(Undefined());
    }

    //TODO: support CFT

    DDS::SubscriberQos sub_qos;
    dp->get_default_subscriber_qos(sub_qos); //TODO: user's sub QoS
    DDS::Subscriber_var sub = dp->create_subscriber(sub_qos, 0, 0);
    if (!sub) {
      ThrowException(Exception::Error(String::New("couldn't create "
                                                  "Subscriber")));
      return scope.Close(Undefined());
    }

    Local<Value> cb = args[args.Length() - 1];
    NodeDRListener* const ndrl = new NodeDRListener(cb.As<Function>());
    DDS::DataReaderListener_var listen(ndrl);
    DDS::DataReaderQos dr_qos;
    sub->get_default_datareader_qos(dr_qos); //TODO: user's dr QoS
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

  // participant.unsubscribe(reader)
  Handle<Value> unsubscribe(const Arguments& args)
  {
    HandleScope scope;
    if (args.Length() < 1 || !args[0]->IsObject()) {
      ThrowException(Exception::TypeError(String::New("1 argument required")));
      return scope.Close(Undefined());
    }
    void* const internal = args.This()->GetPointerFromInternalField(0);
    DDS::DomainParticipant* dp =
      static_cast<DDS::DomainParticipant*>(internal);

    const Local<Object> dr_js = args[0]->ToObject();
    void* const dr_obj = dr_js->GetPointerFromInternalField(0);
    DDS::DataReader_var dr = static_cast<DDS::DataReader*>(dr_obj);
    dr_js->SetPointerInInternalField(0, 0);

    DDS::Subscriber_var sub = dr->get_subscriber();
    DDS::TopicDescription_var td = dr->get_topicdescription();
    dr = 0;
    sub->delete_contained_entities();
    dp->delete_subscriber(sub);

    //TODO: CFT
    DDS::Topic_var topic = DDS::Topic::_narrow(td);
    dp->delete_topic(topic);

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
    DDS::DomainParticipantFactory* ptr =
      static_cast<DDS::DomainParticipantFactory*>(internal);
    DDS::DomainParticipantFactory_var dpf = ptr;
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

  void init_node_opendds(Handle<Object> target)
  {
    node::SetMethod(target, "initialize", initialize);
    node::SetMethod(target, "finalize", finalize);
  }
}

NODE_MODULE(node_opendds, init_node_opendds)
