#include <v8.h>
#include <ace/config-all.h>

#define EXPORT ACE_Proper_Export_Flag

namespace v8 {
  EXPORT Local<Array> Array::New(Isolate*, int) { return {}; }
  EXPORT EscapableHandleScope::EscapableHandleScope(Isolate*) {}
  EXPORT internal::Object** EscapableHandleScope::Escape(internal::Object**) {
    return 0;
  }
  EXPORT void* External::Value() const { return 0; }
  EXPORT internal::Object** HandleScope::CreateHandle(internal::HeapObject*,
                                                      internal::Object*) {
    return 0;
  }
  EXPORT HandleScope::~HandleScope() {}
  EXPORT Local<Integer> Integer::NewFromUnsigned(Isolate*, unsigned int) {
    return {};
  }
  EXPORT Local<Integer> Integer::New(Isolate*, int) { return {}; }
  EXPORT Isolate* Isolate::GetCurrent() { return 0; }
  EXPORT Local<Context> Isolate::GetCurrentContext() { return {}; }
  EXPORT Local<Number> Number::New(Isolate*, double) { return {}; }
  EXPORT Local<Object> Object::New(Isolate*) { return {}; }
  EXPORT bool Object::Set(unsigned int, Local<Value>) { return false; }
  EXPORT bool Object::Set(Local<Value>, Local<Value>) { return false; }
  EXPORT Local<Value> Object::SlowGetInternalField(int) { return {}; }
  EXPORT MaybeLocal<String> String::NewFromTwoByte(Isolate*, const uint16_t*,
                                                   v8::NewStringType, int) {
    return {};
  }
  EXPORT MaybeLocal<String> String::NewFromUtf8(Isolate*, const char*,
                                                v8::NewStringType, int) {
    return {};
  }
  EXPORT void V8::ToLocalEmpty() {}
  EXPORT MaybeLocal<Int32> Value::ToInt32(Local<Context>) const { return {}; }
  EXPORT MaybeLocal<Uint32> Value::ToUint32(Local<Context>) const { return {}; }
}
