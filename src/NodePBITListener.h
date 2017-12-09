#ifndef OPENDDS_NODEPBITLISTENER_H
#define OPENDDS_NODEPBITLISTENER_H

#include <nan.h>

#include <dds/DdsDcpsSubscriptionC.h>

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Registered_Data_Types.h>
#include <dds/DCPS/BuiltInTopicUtils.h>

#include <dds/DCPS/V8TypeConverter.h>
#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/DataReaderImpl.h>

namespace NodeOpenDDS {

  class NodePBITListener
    : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
    , private OpenDDS::DCPS::AbstractSamples {
  public:
    NodePBITListener(const v8::Local<v8::Function>& callback, 
                    const DDS::ParticipantBuiltinTopicDataSeq part_data,
                     const DDS::SampleInfoSeq infos, const DDS::DataReader_var& dr);
    ~NodePBITListener();
    void shutdown();

  private:
    static void async_cb(uv_async_t* async_uv);
    static void close_cb(uv_handle_t* handle_uv);

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

    void on_data_available(DDS::DataReader*);

    void async(); // called from libuv event loop

    Nan::Persistent<v8::Function> callback_;
    const DDS::DataReader_var& dr_;

    DDS::ParticipantBuiltinTopicDataSeq part_data_;
    DDS::SampleInfoSeq infos_;

    struct AsyncUvN : uv_async_t {
      explicit AsyncUvN(NodePBITListener* outer) : outer_(outer) {}
      NodePBITListener* outer_;
    } async_uv_pbit_;

    NodePBITListener(const NodePBITListener&);
    NodePBITListener& operator=(const NodePBITListener&);

   void reserve(CORBA::ULong);
   void push_back(const DDS::SampleInfo& src, const void* sample);

  };

}

#endif
