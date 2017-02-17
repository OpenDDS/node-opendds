#define BUILDING_V8_SHARED
#include <v8.h>

namespace v8 {
  Local<Array> Array::New(Isolate*, int) { return {}; }
  EscapableHandleScope::EscapableHandleScope(Isolate*) {}
  internal::Object** EscapableHandleScope::Escape(internal::Object**) {
    return 0;
  }
  void* External::Value() const { return 0; }
  internal::Object** HandleScope::CreateHandle(internal::HeapObject*,
                                                      internal::Object*) {
    return 0;
  }
  HandleScope::~HandleScope() {}
  Local<Integer> Integer::NewFromUnsigned(Isolate*, unsigned int) {
    return {};
  }
  Local<Integer> Integer::New(Isolate*, int) { return {}; }
  Isolate* Isolate::GetCurrent() { return 0; }
  Local<Context> Isolate::GetCurrentContext() { return {}; }
  Local<Number> Number::New(Isolate*, double) { return {}; }
  Local<Object> Object::New(Isolate*) { return {}; }
  bool Object::Set(unsigned int, Local<Value>) { return false; }
  bool Object::Set(Local<Value>, Local<Value>) { return false; }
  Local<Value> Object::SlowGetInternalField(int) { return {}; }
  MaybeLocal<String> String::NewFromTwoByte(Isolate*, const uint16_t*,
                                                   v8::NewStringType, int) {
    return {};
  }
  MaybeLocal<String> String::NewFromUtf8(Isolate*, const char*,
                                                v8::NewStringType, int) {
    return {};
  }
  void V8::ToLocalEmpty() {}
  MaybeLocal<Int32> Value::ToInt32(Local<Context>) const { return {}; }
  MaybeLocal<Uint32> Value::ToUint32(Local<Context>) const { return {}; }
}
