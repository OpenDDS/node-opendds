#include "NodeDRListener.h"

#include <node.h>

#include <dds/DCPS/DataReaderImpl.h>

namespace NodeOpenDDS {
using namespace v8;

void copyToV8(Object& target, const DDS::Time_t& src)
{
  target.Set(v8::String::NewSymbol("sec"), v8::Integer::New(src.sec));
  target.Set(v8::String::NewSymbol("nanosec"), v8::Integer::New(src.nanosec));
}

void copyToV8(Object& target, const DDS::SampleInfo& src)
{
#define INT(X) target.Set(v8::String::NewSymbol(#X), v8::Integer::New(src.X))
  INT(sample_state);
  INT(view_state);
  INT(instance_state);
  Local<Object> source_timestamp = Object::New();
  copyToV8(**source_timestamp, src.source_timestamp);
  target.Set(v8::String::NewSymbol("source_timestamp"), source_timestamp);
  INT(instance_handle);
  INT(publication_handle);
  INT(disposed_generation_count);
  INT(no_writers_generation_count);
  INT(sample_rank);
  INT(generation_rank);
  INT(absolute_generation_rank);
#undef INT
  target.Set(v8::String::NewSymbol("valid_data"),
             v8::Boolean::New(src.valid_data));
}


NodeDRListener::NodeDRListener(const Local<Function>& callback,
                               const OpenDDS::DCPS::V8TypeConverter& conv)
  : callback_(Persistent<Function>::New(callback))
  , conv_(conv)
  , async_uv_(this)
{
  uv_async_init(uv_default_loop(), &async_uv_, async_cb);
}

NodeDRListener::~NodeDRListener()
{
  uv_close((uv_handle_t*)&async_uv_, 0);
  callback_.Dispose();
  js_dr_.Dispose();
}

void NodeDRListener::async_cb(uv_async_t* async_uv, int /*status*/)
{
  static_cast<AsyncUv*>(async_uv)->outer_->async();
}

void NodeDRListener::on_data_available(DDS::DataReader*)
{
  uv_async_send(&async_uv_);
}

void NodeDRListener::async() // called from libuv event loop
{
  void* const dr_obj = js_dr_->GetPointerFromInternalField(0);
  DDS::DataReader* const dr = static_cast<DDS::DataReader*>(dr_obj);
  OpenDDS::DCPS::DataReaderImpl* const dri =
    dynamic_cast<OpenDDS::DCPS::DataReaderImpl*>(dr);

  OpenDDS::DCPS::DataReaderImpl::GenericBundle gen;
  dri->read_generic(gen, DDS::NOT_READ_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
                    DDS::ANY_INSTANCE_STATE); //TODO: take()

  for (CORBA::ULong i = 0; i < gen.info_.length(); ++i) {
    Handle<Value> argv[] = {js_dr_,
                            v8::Object::New(),
                            v8::Object::New()};
    copyToV8(**argv[1].As<v8::Object>(), gen.info_[i]);
    if (gen.info_[i].valid_data) {
      conv_.toV8(**argv[2].As<v8::Object>(), gen.samples_[i]);
    }
    node::MakeCallback(Context::GetCurrent()->Global(), callback_,
                       sizeof(argv) / sizeof(argv[0]), argv);
  }
}

void NodeDRListener::set_javascript_datareader(const Local<v8::Object>& js_dr)
{
  js_dr_ = Persistent<v8::Object>::New(js_dr);
}


}
