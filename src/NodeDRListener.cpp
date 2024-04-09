#include "NodeDRListener.h"

#include <dds/DCPS/Sample.h>

#include <nan.h>
#include <stdexcept>

namespace NodeOpenDDS {
using namespace v8;

Local<Object> copyToV8(const DDS::Time_t& src)
{
  Local<Object> stru = Nan::New<Object>();
  Nan::Set(stru, Nan::New<String>("sec").ToLocalChecked(), Nan::New(src.sec));
  Nan::Set(stru, Nan::New<String>("nanosec").ToLocalChecked(),
            Nan::New(src.nanosec));
  return stru;
}

Local<Object> copyToV8(const DDS::SampleInfo& src)
{
  Local<Object> stru = Nan::New<Object>();
#define INT(X) Nan::Set(stru, Nan::New<String>(#X).ToLocalChecked(), Nan::New(src.X))
  INT(sample_state);
  INT(view_state);
  INT(instance_state);
  Nan::Set(stru, Nan::New<String>("source_timestamp").ToLocalChecked(),
            copyToV8(src.source_timestamp));
  INT(instance_handle);
  INT(publication_handle);
  INT(disposed_generation_count);
  INT(no_writers_generation_count);
  INT(sample_rank);
  INT(generation_rank);
  INT(absolute_generation_rank);
#undef INT
  Nan::Set(stru, Nan::New<String>("valid_data").ToLocalChecked(),
            Nan::New(src.valid_data));
  return stru;
}

NodeDRListener::NodeDRListener(DDS::DomainParticipant* dp,
                               const Local<Function>& callback)
  : dp_(dp)
  , callback_(callback)
  , vd_(0)
  , async_uv_(this)
  , unsubscribing_(false)
  , unsubscribed_(false)
  , receiving_samples_(false)
{
  uv_async_init(uv_default_loop(), &async_uv_, async_cb);
}

NodeDRListener::~NodeDRListener()
{
}

void NodeDRListener::async_cb(uv_async_t* async_uv)
{
  static_cast<AsyncUv*>(async_uv)->outer_->async();
}

void NodeDRListener::close_cb(uv_handle_t* handle_uv)
{
  static_cast<AsyncUv*>((uv_async_t*)handle_uv)->outer_->_remove_ref();
}

void NodeDRListener::on_data_available(DDS::DataReader*)
{
  std::unique_lock<std::mutex> lock(mutex_);
  if (unsubscribing_) {
    return;
  }

  uv_async_send(&async_uv_);
}

void NodeDRListener::async() // called from libuv event loop
{
  std::unique_lock<std::mutex> lock(mutex_);
  receiving_samples_ = true;

  Nan::HandleScope scope;
  Local<v8::Object> js_dr = Nan::New(js_dr_);
  void* const dr_obj = Nan::GetInternalFieldPointer(js_dr, 0);
  DDS::DataReader* const dr = static_cast<DDS::DataReader*>(dr_obj);
  OpenDDS::DCPS::DataReaderImpl* const dri =
    dynamic_cast<OpenDDS::DCPS::DataReaderImpl*>(dr);

  lock.unlock();
  try {
    OpenDDS::DCPS::DataReaderImpl::GenericBundle gen;
    dri->take(*this, DDS::NOT_READ_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
              DDS::ANY_INSTANCE_STATE);
  } catch (...) {
  }
  lock.lock();

  receiving_samples_ = false;
  if (unsubscribing_) {
    // In a new event after this, we can safely unsubscribe
    Nan::AsyncQueueWorker(new UnsubscribeWorker(this));
  }
}

void NodeDRListener::set_javascript_datareader(const Local<v8::Object>& js_dr)
{
  std::unique_lock<std::mutex> lock(mutex_);
  js_dr_.Reset(js_dr);
  void* const dr_obj = Nan::GetInternalFieldPointer(js_dr, 0);
  OpenDDS::DCPS::DataReaderImpl* dri = dynamic_cast<OpenDDS::DCPS::DataReaderImpl*>(static_cast<DDS::DataReader*>(dr_obj));
  if (dri) {
    vd_ = dri->get_value_dispatcher();
  }
}

void NodeDRListener::reserve(CORBA::ULong)
{
}

void NodeDRListener::push_back(const DDS::SampleInfo& src, const void* sample)
{
  std::unique_lock<std::mutex> lock(mutex_);
  if (unsubscribing_) {
    return;
  }

  const Sample::Extent ext = src.valid_data ? Sample::Full : Sample::KeyOnly;

  if (vd_) {
    if (!vd_->write(nvw_, sample, ext)) {
      ACE_ERROR((LM_WARNING, "WARNING: ValueDispatcher write failed\n"));
      return;
    }
  }

  Local<Value> argv[] = {
    Nan::New(js_dr_),
    copyToV8(src),
    vd_ ? nvw_.get_result().As<Value>() : Nan::Undefined().As<Value>()
  };

  Local<Function> callback = Nan::New(callback_);
  lock.unlock();

  Nan::Callback cb(callback);
  Nan::Call(cb, sizeof(argv) / sizeof(argv[0]), argv);
}

void NodeDRListener::unsubscribe()
{
  std::unique_lock<std::mutex> lock(mutex_);
  if (receiving_samples_) {
    // Inform the Listener to skip any remaining samples and use
    // UnsubscibeWorker to unsubscribe at the next opportunity.
    unsubscribing_ = true;
  } else { // Unsubscribe Now
    unsubscribe_now_i(lock);
  }
}

void NodeDRListener::unsubscribe_now()
{
  std::unique_lock<std::mutex> lock(mutex_);
  unsubscribe_now_i(lock);
}

void NodeDRListener::unsubscribe_now_i(std::unique_lock<std::mutex>& lock)
{
  if (unsubscribed_) {
    return;
  }

  Local<v8::Object> dr_js = Nan::New(js_dr_);
  void* const dr_obj = Nan::GetInternalFieldPointer(dr_js, 0);
  DDS::DataReader_var dr = static_cast<DDS::DataReader*>(dr_obj);
  Nan::SetInternalFieldPointer(dr_js, 0, 0);

  // Keep alive at least until after this method
  _add_ref();
  uv_close((uv_handle_t*)&async_uv_, close_cb);

  const DDS::DomainParticipant_var dp(DDS::DomainParticipant::_duplicate(dp_));
  const DDS::Subscriber_var sub = dr->get_subscriber();
  const DDS::TopicDescription_var td = dr->get_topicdescription();

  dp_ = 0;
  unsubscribed_ = true;

  lock.unlock();

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
}

UnsubscribeWorker::UnsubscribeWorker(NodeDRListener* ndrl) : AsyncWorker(NULL)
{
  ndrl_ = ndrl;
}

UnsubscribeWorker::~UnsubscribeWorker()
{
}

void UnsubscribeWorker::Execute()
{
}

void UnsubscribeWorker::Destroy()
{
}

void UnsubscribeWorker::HandleOKCallback()
{
  ndrl_->unsubscribe_now();
}
void UnsubscribeWorker::HandleErrorCallback()
{
}


}
