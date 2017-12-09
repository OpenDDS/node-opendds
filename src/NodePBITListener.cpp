#include "NodePBITListener.h"

#include <dds/DCPS/Service_Participant.h>
#include <dds/DCPS/Registered_Data_Types.h>
#include <dds/DCPS/BuiltInTopicUtils.h>

#include <nan.h>
#include <stdexcept>

namespace NodeOpenDDS {
using namespace v8;

Local<Object> copytoV8(const DDS::Time_t& src)
{
  Local<Object> stru = Nan::New<Object>();
  stru->Set(Nan::New<String>("sec").ToLocalChecked(), Nan::New(src.sec));
  stru->Set(Nan::New<String>("nanosec").ToLocalChecked(),
            Nan::New(src.nanosec));
  return stru;
}

Local<Object> copytoV8(const DDS::SampleInfo& src)
{
  Local<Object> stru = Nan::New<Object>();
#define INT(X) stru->Set(Nan::New<String>(#X).ToLocalChecked(), Nan::New(src.X))
  INT(sample_state);
  INT(view_state);
  INT(instance_state);
  stru->Set(Nan::New<String>("source_timestamp").ToLocalChecked(),
            copytoV8(src.source_timestamp));
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

Local<Object> toV8(const DDS::ParticipantBuiltinTopicData& src)
{
  ACE_UNUSED_ARG(src);
  Local<Object> stru = Nan::New<Object>();

  std::string str;
  for (CORBA::ULong i = 0; i < src.user_data.value.length(); ++i) {
    str += src.user_data.value[i];
  }
  stru->Set(Nan::New<v8::String>("user_data").ToLocalChecked(), Nan::New(str).ToLocalChecked());
  
  const v8::Local<v8::Array> tgt(Nan::New<v8::Array>(3));
  for (CORBA::Long i = 0; i < 3; ++i) {
    tgt->Set(Nan::New(i), Nan::New(src.key.value[i]));
  }
  stru->Set(Nan::New<v8::String>("key").ToLocalChecked(), tgt);

  return stru;
}

NodePBITListener::NodePBITListener(const Local<Function>& callback,
                                  const DDS::ParticipantBuiltinTopicDataSeq part_data,
                                  const DDS::SampleInfoSeq infos,
                                  const DDS::DataReader_var& dr)
  : callback_(callback)
  , part_data_(part_data)
  , infos_(infos)
  , dr_(dr)
  , async_uv_pbit_(this)
{
  uv_async_init(uv_default_loop(), &async_uv_pbit_, async_cb);
}

NodePBITListener::~NodePBITListener()
{
}

void NodePBITListener::async_cb(uv_async_t* async_uv)
{
  static_cast<AsyncUvN*>(async_uv)->outer_->async();
}

void NodePBITListener::close_cb(uv_handle_t* handle_uv)
{
  static_cast<AsyncUvN*>((uv_async_t*)handle_uv)->outer_->_remove_ref();
}

void NodePBITListener::shutdown()
{
  _add_ref();
  uv_close((uv_handle_t*)&async_uv_pbit_, close_cb);
}

void NodePBITListener::on_data_available(DDS::DataReader* dr)
{

  DDS::ParticipantBuiltinTopicDataDataReader_var part_dr =
        DDS::ParticipantBuiltinTopicDataDataReader::_narrow(dr);
  
  part_dr->take(part_data_, infos_, 1, DDS::NOT_READ_SAMPLE_STATE, DDS::ANY_VIEW_STATE,
            DDS::ANY_INSTANCE_STATE);
  
  uv_async_send(&async_uv_pbit_);
}

void NodePBITListener::async() // called from libuv event loop
{
  Nan::HandleScope scope;
  
  try {
    const v8::Local<v8::Object> stru = Nan::New<v8::Object>();
    
    Local<Value> argv[] = { copytoV8(infos_[0]), toV8(part_data_[0]) };

    Local<Function> callback = Nan::New(callback_);
    Nan::Callback cb(callback);
    cb.Call(sizeof(argv) / sizeof(argv[0]), argv);
  } catch (...) {
  }
}

void NodePBITListener::reserve(CORBA::ULong)
{
}

void NodePBITListener::push_back(const DDS::SampleInfo& src, const void* sample)
{
}

}
