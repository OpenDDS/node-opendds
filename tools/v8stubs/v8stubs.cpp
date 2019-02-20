#define BUILDING_V8_SHARED
#define V8_SHARED
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
  int String::WriteUtf8(char*, int, int*, int) const { return 0; }
  bool Value::IsArray() const { return false; }
  MaybeLocal<Number> Value::ToNumber(v8::Local<v8::Context>) const { return {}; }
  Local<Value> Object::Get(Handle<v8::Value>) { return {}; }
  bool Value::IsBoolean() const { return false; }
  bool Value::IsNumber() const { return false; }
  int String::Length() const { return 0; }
  int String::Utf8Length() const { return 0; }
  int64_t Value::IntegerValue() const { return 0; }
  double Number::Value() const { return 0.0; }
  bool Boolean::Value() const { return false; }
  int64_t Integer::Value() const { return false; }
  bool Object::Has(v8::Local<v8::Value>) { return false; }
  MaybeLocal<String> Value::ToString(v8::Local<v8::Context>) const { return {}; }
  int v8::String::Write(unsigned short*, int, int, int) const { return 0; }
  MaybeLocal<Object> Value::ToObject(v8::Local<v8::Context>) const { return {}; }
  MaybeLocal<Boolean> Value::ToBoolean(v8::Local<v8::Context>) const { return {}; }
  MaybeLocal<Integer> Value::ToInteger(v8::Local<v8::Context>) const { return {}; }
  Local<Value> Object::Get(unsigned int) { return {}; }
  uint32_t Value::Uint32Value() const { return 0; }
}
