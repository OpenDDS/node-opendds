#include "NodeJSTestV8.h"
#include "NodeJSTestTypeSupportImpl.h"

#include "dds/DCPS/V8TypeConverter.h"

#include <v8.h>

namespace Mod {

  void copyToV8(v8::Object& target, const Sample& src)
  {
    target.Set(v8::String::NewSymbol("id"), v8::Integer::New(src.id));
    target.Set(v8::String::NewSymbol("data"), v8::String::New(src.data.in()));
  }

  class SampleTypeSupportV8Impl
    : public virtual SampleTypeSupportImpl
    , public virtual OpenDDS::DCPS::V8TypeConverter {

    void toV8(v8::Object& target, const void* source) const
    {
      copyToV8(target, *static_cast<const Sample*>(source));
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
