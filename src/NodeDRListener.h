#ifndef OPENDDS_NODEDRLISTENER_H
#define OPENDDS_NODEDRLISTENER_H

#include <nan.h>

#include "NodeValueWriter.h"

#include <dds/DdsDcpsSubscriptionC.h>

#include <dds/DCPS/LocalObject.h>
#include <dds/DCPS/DataReaderImpl.h>

#include <mutex>

namespace NodeOpenDDS {

  class NodeDRListener
    : public virtual OpenDDS::DCPS::LocalObject<DDS::DataReaderListener>
    , private OpenDDS::DCPS::AbstractSamples {
  public:
    NodeDRListener(DDS::DomainParticipant* dp,
                   const v8::Local<v8::Function>& callback);
    ~NodeDRListener();

    void set_javascript_datareader(const v8::Local<v8::Object>& js_dr);

    /**
     * If receiving samples, ignore any more samples and unsubscribe
     * afterwards, else unsubscribe now.
     */
    void unsubscribe();

    /// Unsubscribe immediately
    void unsubscribe_now();

  private:
    static void async_cb(uv_async_t* async_uv);
    static void close_cb(uv_handle_t* handle_uv);

    void unsubscribe_now_i(std::unique_lock<std::mutex>& lock);

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

    DDS::DomainParticipant* dp_;
    Nan::Persistent<v8::Function> callback_;
    Nan::Persistent<v8::Object> js_dr_;
    const OpenDDS::DCPS::ValueDispatcher* vd_;
    NodeValueWriter nvw_;

    struct AsyncUv : uv_async_t {
      explicit AsyncUv(NodeDRListener* outer) : outer_(outer) {}
      NodeDRListener* outer_;
    } async_uv_;

    NodeDRListener(const NodeDRListener&);
    NodeDRListener& operator=(const NodeDRListener&);

    void reserve(CORBA::ULong);
    void push_back(const DDS::SampleInfo& src, const void* sample);

    /// True if Datareader Listener is going to unsubscribe itself soon.
    bool unsubscribing_;
    bool unsubscribed_;

    /// True if Datareader Listener taking samples.
    bool receiving_samples_;

    unsigned long count_samples_;

    mutable std::mutex mutex_;
  };

  /// Wrapper object to call unsubscribe at a better time using Node Event
  /// Loop.
  class UnsubscribeWorker : public Nan::AsyncWorker {
  public:
    UnsubscribeWorker(NodeDRListener* ndrl);
    ~UnsubscribeWorker();

    void Execute();
    void Destroy();

  protected:
    NodeDRListener* ndrl_;

    void HandleOKCallback();
    void HandleErrorCallback();
  };

}

#endif
