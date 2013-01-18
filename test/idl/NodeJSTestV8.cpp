#include "NodeJSTestV8.h"
#include "NodeJSTestTypeSupportImpl.h"

#include "dds/DCPS/V8TypeConverter.h"

#include <v8.h>

namespace Mod {

  v8::Value* copyToV8(const Sample& src)
  {
    v8::Local<v8::Object> stru = v8::Object::New();
    stru->Set(v8::String::NewSymbol("id"), v8::Integer::New(src.id));
    stru->Set(v8::String::NewSymbol("data"), v8::String::New(src.data.in()));
    return *stru;
  }

  class SampleTypeSupportV8Impl
    : public virtual SampleTypeSupportImpl
    , public virtual OpenDDS::DCPS::V8TypeConverter {

    v8::Value* toV8(const void* source) const
    {
      return copyToV8(*static_cast<const Sample*>(source));
    }

  public:
    struct Initializer {
      Initializer()
      {
        SampleTypeSupport_var ts = new SampleTypeSupportV8Impl;
        ts->register_type(0, "");
      }
    };
  };

  SampleTypeSupportV8Impl::Initializer init;
}
