#include "NodeDRListener.h"

#include <nan.h>
#include <stdexcept>

namespace NodeOpenDDS {
using namespace v8;

Local<Object> copyToV8(const DDS::Time_t& src)
{
  Local<Object> stru = Nan::New<Object>();
  stru->Set(Nan::New<String>("sec").ToLocalChecked(), Nan::New(src.sec));
  stru->Set(Nan::New<String>("nanosec").ToLocalChecked(),
            Nan::New(src.nanosec));
  return stru;
}

Local<Object> copyToV8(const DDS::SampleInfo& src)
{
  Local<Object> stru = Nan::New<Object>();
#define INT(X) stru->Set(Nan::New<String>(#X).ToLocalChecked(), Nan::New(src.X))
  INT(sample_state);
  INT(view_state);
  INT(instance_state);
  stru->Set(Nan::New<String>("source_timestamp").ToLocalChecked(),
            copyToV8(src.source_timestamp));
  INT(instance_handle);
  INT(publication_handle);
  INT(disposed_generation_count);
  INT(no_writers_generation_count);
  INT(sample_rank);
  INT(generation_rank);
  INT(absolute_generation_rank);
#undef INT
  stru->Set(Nan::New<String>("valid_data").ToLocalChecked(),
            Nan::New(src.valid_data));
  return stru;
}

NodeDRListener::NodeDRListener(const Local<Function>& callback,
                               const OpenDDS::DCPS::V8TypeConverter& conv)
  : callback_(callback)
  , conv_(conv)
  , async_uv_(this)
  , unsubscribing_(false)
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

void NodeDRListener::shutdown()
{
  _add_ref();
  uv_close((uv_handle_t*)&async_uv_, close_cb);
}

void NodeDRListener::on_data_available(DDS::DataReader*)
{
  if (unsubscribing_) {
    return;
  }

  uv_async_send(&async_uv_);
}

void NodeDRListener::async() // called from libuv event loop
{
  Nan::HandleScope scope;
  Local<v8::Object> js_dr = Nan::New(js_dr_);
  void* const dr_obj = Nan::GetInternalFieldPointer(js_dr, 0);
  DDS::DataReader* const dr = static_cast<DDS::DataReader*>(dr_obj);
  OpenDDS::DCPS::DataReaderImpl* const dri =
    dynamic_cast<OpenDDS::DCPS::DataReaderImpl*>(dr);

  try {
    OpenDDS::DCPS::DataReaderImpl::GenericBundle gen;
    dri->take(*this, DDS::NOT_READ_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
              DDS::ANY_INSTANCE_STATE);
  } catch (...) {
  }
}

void NodeDRListener::set_javascript_datareader(const Local<v8::Object>& js_dr)
{
  js_dr_.Reset(js_dr);
}

void NodeDRListener::reserve(CORBA::ULong)
{
}

void NodeDRListener::push_back(const DDS::SampleInfo& src, const void* sample)
{
  if (unsubscribing_) {
    return;
  }

  Local<Value> argv[] = {Nan::New(js_dr_), Handle<Value>(copyToV8(src)),
                         src.valid_data ? conv_.toV8(sample).As<Value>()
                         : Nan::Undefined().As<Value>()};

  Local<Function> callback = Nan::New(callback_);
  Nan::Callback cb(callback);
  cb.Call(sizeof(argv) / sizeof(argv[0]), argv);

  // check for the case of unsubscribe inside the callback
  if (!Nan::GetInternalFieldPointer(argv[0].As<v8::Object>(), 0)) {
    throw std::runtime_error("invalid Javascript Data Reader callback object");
  }
}

void NodeDRListener::unsubscribing() {
  unsubscribing_ = true;
}

}
