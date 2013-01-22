#include "NodeDRListener.h"

#include <node.h>

#include <dds/DCPS/DataReaderImpl.h>

namespace NodeOpenDDS {
using namespace v8;

Value* copyToV8(const DDS::Time_t& src)
{
  Local<Object> stru = Object::New();
  stru->Set(String::NewSymbol("sec"), Integer::New(src.sec));
  stru->Set(String::NewSymbol("nanosec"), Integer::New(src.nanosec));
  return *stru;
}

Value* copyToV8(const DDS::SampleInfo& src)
{
  Local<Object> stru = Object::New();
#define INT(X) stru->Set(String::NewSymbol(#X), Integer::New(src.X))
  INT(sample_state);
  INT(view_state);
  INT(instance_state);
  stru->Set(String::NewSymbol("source_timestamp"),
            Handle<Value>(copyToV8(src.source_timestamp)));
  INT(instance_handle);
  INT(publication_handle);
  INT(disposed_generation_count);
  INT(no_writers_generation_count);
  INT(sample_rank);
  INT(generation_rank);
  INT(absolute_generation_rank);
#undef INT
  stru->Set(String::NewSymbol("valid_data"), Boolean::New(src.valid_data));
  return *stru;
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
  
  callback_.Dispose();
  js_dr_.Dispose();
}

void NodeDRListener::async_cb(uv_async_t* async_uv, int /*status*/)
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
  uv_async_send(&async_uv_);
}

void NodeDRListener::async() // called from libuv event loop
{
  HandleScope scope;
  void* const dr_obj = js_dr_->GetPointerFromInternalField(0);
  DDS::DataReader* const dr = static_cast<DDS::DataReader*>(dr_obj);
  OpenDDS::DCPS::DataReaderImpl* const dri =
    dynamic_cast<OpenDDS::DCPS::DataReaderImpl*>(dr);

  OpenDDS::DCPS::DataReaderImpl::GenericBundle gen;
  dri->read_generic(gen, DDS::NOT_READ_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
                    DDS::ANY_INSTANCE_STATE);

  for (CORBA::ULong i = 0; i < gen.info_.length(); ++i) {
    Handle<Value> argv[] = {js_dr_, Handle<Value>(copyToV8(gen.info_[i])),
                            Undefined()};
    if (gen.info_[i].valid_data) {
      argv[2] = Handle<Value>(conv_.toV8(gen.samples_[i]));
    }
    node::MakeCallback(Context::GetCurrent()->Global(), callback_,
                       sizeof(argv) / sizeof(argv[0]), argv);
  }

  if (js_dr_->GetPointerFromInternalField(0)) { // in case of unsubscribe in cb
    dri->take_generic(DDS::READ_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
                      DDS::ANY_INSTANCE_STATE);
  }
}

void NodeDRListener::set_javascript_datareader(const Local<v8::Object>& js_dr)
{
  js_dr_ = Persistent<v8::Object>::New(js_dr);
}


}
