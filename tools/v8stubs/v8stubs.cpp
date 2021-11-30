#define BUILDING_V8_SHARED
#define V8_SHARED
#include <v8.h>

namespace v8 {
  Local<Array> Array::New(Isolate*, int) { return {}; }
  EscapableHandleScope::EscapableHandleScope(Isolate*) {}
  void* External::Value() const { return 0; }

#if (V8_MAJOR_VERSION < 7)
  internal::Object** EscapableHandleScope::Escape(internal::Object**) {
    return 0;
  }
  internal::Object** HandleScope::CreateHandle(internal::HeapObject*, internal::Object*) {
    return 0;
  }
  int String::WriteUtf8(char*, int, int*, int) const { return 0; }
#else
  Local<Boolean> Value::ToBoolean(v8::Isolate*) const { return {}; }
  internal::Address* EscapableHandleScope::Escape(internal::Address*) { return 0; }
  internal::Address* HandleScope::CreateHandle(v8::internal::Isolate*, unsigned long) { return 0; }
  namespace internal {
    Isolate* IsolateFromNeverReadOnlySpaceObject(Address) { return 0; }
  }
  int String::WriteUtf8(Isolate*, char*, int, int*, int) const { return 0; }
#endif

#if (V8_MAJOR_VERSION < 8)
  bool Object::Set(unsigned int, Local<Value>) { return false; }
  bool Object::Set(Local<Value>, Local<Value>) { return false; }
  MaybeLocal<Boolean> Value::ToBoolean(v8::Local<v8::Context>) const { return {}; }
  Local<Value> Object::Get(Handle<v8::Value>) { return {}; }
  Local<Value> Object::Get(unsigned int) { return {}; }
#else
#endif

  bool Value::IsBoolean() const { return false; }
  bool Boolean::Value() const { return false; }

  HandleScope::~HandleScope() {}
  Local<Integer> Integer::NewFromUnsigned(Isolate*, unsigned int) {
    return {};
  }
  Local<Integer> Integer::New(Isolate*, int) { return {}; }
  Isolate* Isolate::GetCurrent() { return 0; }
  Local<Context> Isolate::GetCurrentContext() { return {}; }
  Local<Number> Number::New(Isolate*, double) { return {}; }
  Local<Object> Object::New(Isolate*) { return {}; }
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
  bool Value::IsArray() const { return false; }
  MaybeLocal<Number> Value::ToNumber(v8::Local<v8::Context>) const { return {}; }
  bool Value::IsNumber() const { return false; }
  int String::Length() const { return 0; }
  double Number::Value() const { return 0.0; }
  int64_t Integer::Value() const { return 0l; }
  MaybeLocal<String> Value::ToString(v8::Local<v8::Context>) const { return {}; }
  MaybeLocal<Object> Value::ToObject(v8::Local<v8::Context>) const { return {}; }
  MaybeLocal<Integer> Value::ToInteger(v8::Local<v8::Context>) const { return {}; }
  Maybe<bool> Object::Has(v8::Local<v8::Context>, v8::Local<v8::Value>) { return Just(false); }
  MaybeLocal<Value> Object::Get(v8::Local<v8::Context>, v8::Local<v8::Value>) { return {}; }
  Maybe<bool> Object::Set(v8::Local<v8::Context>, unsigned int, v8::Local<v8::Value>) { return Just(false); }
  Maybe<uint32_t> Value::Uint32Value(v8::Local<v8::Context>) const { return Just(0u); }
  Maybe<bool> Object::Set(v8::Local<v8::Context>, v8::Local<v8::Value>, v8::Local<v8::Value>) { return Just(false); }
  MaybeLocal<Value> Object::Get(v8::Local<v8::Context>, unsigned int) { return {}; }
  Maybe<int64_t> Value::IntegerValue(v8::Local<v8::Context>) const { return Just(0l); }
  HandleScope::HandleScope(v8::Isolate*) {}
}
