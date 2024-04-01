#include "NodeValueWriter.h"

#if NODE_MAJOR_VERSION > 10 || (NODE_MAJOR_VERSION == 10 && NODE_MINOR_VERSION >= 4)
#define HAS_BIGINT
#endif

namespace NodeOpenDDS {

namespace {
  const int64_t NODE_MAX_SAFE_INT = 9007199254740991;
}

NodeValueWriter::NodeValueWriter()
  : next_index_(0)
  , use_bigint_(true)
{
}

void NodeValueWriter::use_bigint(bool value)
{
  use_bigint_ = value;
}

bool NodeValueWriter::begin_struct(OpenDDS::DCPS::Extensibility /*extensibility*/)
{
  return object_helper<v8::Object>();
}

bool NodeValueWriter::end_struct()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
  return true;
}

bool NodeValueWriter::begin_struct_member(OpenDDS::DCPS::MemberParam params)
{
  next_key_ = params.name;
  return true;
}

bool NodeValueWriter::end_struct_member()
{
  return true;
}

bool NodeValueWriter::begin_union(OpenDDS::DCPS::Extensibility /*extensibility*/)
{
  return object_helper<v8::Object>();
}

bool NodeValueWriter::end_union()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
  return true;
}

bool NodeValueWriter::begin_discriminator(OpenDDS::DCPS::MemberParam /*params*/)
{
  next_key_ = "$discriminator";
  return true;
}

bool NodeValueWriter::end_discriminator()
{
  return true;
}

bool NodeValueWriter::begin_union_member(OpenDDS::DCPS::MemberParam params)
{
  next_key_ = params.name;
  return true;
}

bool NodeValueWriter::end_union_member()
{
  return true;
}

bool NodeValueWriter::begin_array(OpenDDS::XTypes::TypeKind /*elem_kind*/)
{
  return object_helper<v8::Array>();
}

bool NodeValueWriter::end_array()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
  return true;
}

bool NodeValueWriter::begin_sequence(OpenDDS::XTypes::TypeKind /*elem_kind*/, ACE_CDR::ULong /*length*/)
{
  return object_helper<v8::Array>();
}

bool NodeValueWriter::end_sequence()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
  return true;
}

bool NodeValueWriter::begin_element(ACE_CDR::ULong idx)
{
  next_index_ = idx;
  return true;
}

bool NodeValueWriter::end_element()
{
  return true;
}

bool NodeValueWriter::write_boolean(ACE_CDR::Boolean value)
{
  return primitive_helper<ACE_CDR::Boolean, v8::Boolean>(value);
}

bool NodeValueWriter::write_byte(ACE_CDR::Octet value)
{
  return primitive_helper<ACE_CDR::Octet, v8::Integer>(value);
}

#if OPENDDS_HAS_EXPLICIT_INTS

bool NodeValueWriter::write_int8(ACE_CDR::Int8 value)
{
  return primitive_helper<ACE_CDR::Int8, v8::Integer>(value);
}

bool NodeValueWriter::write_uint8(ACE_CDR::UInt8 value)
{
  return primitive_helper<ACE_CDR::UInt8, v8::Integer>(value);
}
#endif

bool NodeValueWriter::write_int16(ACE_CDR::Short value)
{
  return primitive_helper<ACE_CDR::Short, v8::Integer>(value);
}

bool NodeValueWriter::write_uint16(ACE_CDR::UShort value)
{
  return primitive_helper<ACE_CDR::UShort, v8::Integer>(value);
}

bool NodeValueWriter::write_int32(ACE_CDR::Long value)
{
  return primitive_helper<ACE_CDR::Long, v8::Integer>(value);
}

bool NodeValueWriter::write_uint32(ACE_CDR::ULong value)
{
  return primitive_helper<ACE_CDR::ULong, v8::Integer>(value);
}

bool NodeValueWriter::write_int64(ACE_CDR::LongLong value)
{
  if (value <= NODE_MAX_SAFE_INT) {
    return primitive_helper<ACE_CDR::LongLong, v8::Number>(value);
  }

  bool ret = true;
#ifdef HAS_BIGINT
  if (use_bigint_) {
    v8::MaybeLocal<v8::BigInt> v8_value = v8::BigInt::New(v8::Isolate::GetCurrent(), static_cast<int64_t>(value));
    ret = value_helper<v8::BigInt>(v8_value);
  } else {
#endif
    char buff[21]; // 2^63 is 19 characters long in decimal representation, plus optional sign, plus null
    std::sprintf(buff, ACE_INT64_FORMAT_SPECIFIER_ASCII, value);
    v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(buff, std::strlen(buff));
    ret = value_helper<v8::String>(v8_value);
#ifdef HAS_BIGINT
  }
#endif

  return ret;
}

bool NodeValueWriter::write_uint64(ACE_CDR::ULongLong value)
{
  if (value <= static_cast<ACE_CDR::ULongLong>(NODE_MAX_SAFE_INT)) {
    return primitive_helper<ACE_CDR::ULongLong, v8::Number>(value);
  }

  bool ret = true;
#ifdef HAS_BIGINT
  if (use_bigint_) {
    v8::MaybeLocal<v8::BigInt> v8_value = v8::BigInt::NewFromUnsigned(v8::Isolate::GetCurrent(), static_cast<uint64_t>(value));
    ret = value_helper<v8::BigInt>(v8_value);
  } else {
#endif
    char buff[21]; // 2^64 is 20 characters long in decimal representation, plus null
    std::sprintf(buff, ACE_UINT64_FORMAT_SPECIFIER_ASCII, value);
    v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(buff, std::strlen(buff));
    ret = value_helper<v8::String>(v8_value);
#ifdef HAS_BIGINT
  }
#endif

  return ret;
}

bool NodeValueWriter::write_float32(ACE_CDR::Float value)
{
  return primitive_helper<ACE_CDR::Float, v8::Number>(value);
}

bool NodeValueWriter::write_float64(ACE_CDR::Double value)
{
  return primitive_helper<ACE_CDR::Double, v8::Number>(value);
}

bool NodeValueWriter::write_float128(ACE_CDR::LongDouble value)
{
#if ACE_SIZEOF_LONG_DOUBLE == 16
  return primitive_helper<ACE_CDR::LongDouble, v8::Number>(value);
#else
  return primitive_helper<ACE_CDR::LongDouble::NativeImpl, v8::Number>(value);
#endif
}

bool NodeValueWriter::write_fixed(const ACE_CDR::Fixed& /*value*/)
{
  return true;
}

bool NodeValueWriter::write_char8(ACE_CDR::Char value)
{
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(&value, 1);
  return value_helper<v8::String>(v8_value);
}

bool NodeValueWriter::write_char16(ACE_CDR::WChar value)
{
  const uint16_t c = static_cast<uint16_t>(value);
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(&c, 1);
  return value_helper<v8::String>(v8_value);
}

bool NodeValueWriter::write_string(const ACE_CDR::Char* value, size_t length)
{
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(value, length);
  return value_helper<v8::String>(v8_value);
}

bool NodeValueWriter::write_wstring(const ACE_CDR::WChar* value, size_t length)
{
  uint16_t* const str = new uint16_t[length];
  for (size_t i = 0; i < length; ++i) {
    str[i] = static_cast<uint16_t>(value[i]);
  }
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(str, length);
  delete [] str;
  return value_helper<v8::String>(v8_value);
}

bool NodeValueWriter::write_enum(ACE_CDR::Long value, const OpenDDS::DCPS::EnumHelper& helper)
{
  const char* name = 0;
  if (!helper.get_name(name, value)) {
    return false;
  }
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(name);
  return value_helper<v8::String>(v8_value);
}

bool NodeValueWriter::write_bitmask(ACE_CDR::ULongLong value, const OpenDDS::DCPS::BitmaskHelper& helper)
{
  OPENDDS_VECTOR(OpenDDS::DCPS::String) flags;
  helper.get_names(flags, value);
  v8::MaybeLocal<v8::Object> array = Nan::New<v8::Array>();
  if (array.IsEmpty()) {
    return false;
  }

  uint32_t i = 0;
  for (OPENDDS_VECTOR(OpenDDS::DCPS::String)::const_iterator it = flags.begin();
       it != flags.end(); ++it, ++i) {
    v8::MaybeLocal<v8::String> flag = Nan::New<v8::String>(*it);
    if (flag.IsEmpty()) {
      return false;
    }
    Nan::Set(array.ToLocalChecked(), i, flag.ToLocalChecked());
  }

  if (object_stack_.empty()) {
    return false;
  }

  insert_object(array);
  return true;
}

bool NodeValueWriter::write_absent_value()
{
  if (object_stack_.empty()) {
    return false;
  }

  v8::Local<v8::Primitive> null_value = Nan::Null();
  v8::MaybeLocal<v8::String> key = Nan::New<v8::String>(next_key_.c_str());
  if (!key.IsEmpty()) {
    Nan::Set(object_stack_.back(), key.ToLocalChecked(), null_value);
    return true;
  }
  return false;
}

} // NodeOpenDDS
