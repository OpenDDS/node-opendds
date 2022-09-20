#include "NodeValueWriter.h"

namespace NodeOpenDDS {

namespace {
  const int64_t NODE_MAX_SAFE_INT = 9007199254740991;
}

NodeValueWriter::NodeValueWriter() : next_index_(0)
{
}

void NodeValueWriter::begin_struct()
{
  object_helper<v8::Object>();
}

void NodeValueWriter::end_struct()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
}

void NodeValueWriter::begin_struct_member(const DDS::MemberDescriptor& descriptor)
{
  next_key_ = descriptor.name();
}

void NodeValueWriter::end_struct_member()
{
}

void NodeValueWriter::begin_union()
{
  object_helper<v8::Object>();
}

void NodeValueWriter::end_union()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
}

void NodeValueWriter::begin_discriminator()
{
  next_key_ = "$discriminator";
}

void NodeValueWriter::end_discriminator()
{
}

void NodeValueWriter::begin_union_member(const char* name)
{
  next_key_ = name;
}

void NodeValueWriter::end_union_member()
{
}

void NodeValueWriter::begin_array()
{
  object_helper<v8::Array>();
}

void NodeValueWriter::end_array()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
}

void NodeValueWriter::begin_sequence()
{
  object_helper<v8::Array>();
}

void NodeValueWriter::end_sequence()
{
  if (object_stack_.size() == 1) {
    result_ = object_stack_.back();
  }
  object_stack_.pop_back();
}

void NodeValueWriter::begin_element(size_t idx)
{
  next_index_ = idx;
}

void NodeValueWriter::end_element()
{
}

void NodeValueWriter::write_boolean(ACE_CDR::Boolean value)
{
  primitive_helper<ACE_CDR::Boolean, v8::Boolean>(value);
}

void NodeValueWriter::write_byte(ACE_CDR::Octet value)
{
  primitive_helper<ACE_CDR::Octet, v8::Integer>(value);
}

#if OPENDDS_HAS_EXPLICIT_INTS

void NodeValueWriter::write_int8(ACE_CDR::Int8 value)
{
  primitive_helper<ACE_CDR::Int8, v8::Integer>(value);
}

void NodeValueWriter::write_uint8(ACE_CDR::UInt8 value)
{
  primitive_helper<ACE_CDR::UInt8, v8::Integer>(value);
}
#endif

void NodeValueWriter::write_int16(ACE_CDR::Short value)
{
  primitive_helper<ACE_CDR::Short, v8::Integer>(value);
}

void NodeValueWriter::write_uint16(ACE_CDR::UShort value)
{
  primitive_helper<ACE_CDR::UShort, v8::Integer>(value);
}

void NodeValueWriter::write_int32(ACE_CDR::Long value)
{
  primitive_helper<ACE_CDR::Long, v8::Integer>(value);
}

void NodeValueWriter::write_uint32(ACE_CDR::ULong value)
{
  primitive_helper<ACE_CDR::ULong, v8::Integer>(value);
}

void NodeValueWriter::write_int64(ACE_CDR::LongLong value)
{
  if (value <= NODE_MAX_SAFE_INT) {
    primitive_helper<ACE_CDR::LongLong, v8::Number>(value);
    return;
  }

  // If we decide not to use BigInt
  char buff[21]; // 2^63 is 19 characters long in decimal representation, plus optional sign, plus null
#ifndef ACE_LINUX
  std::sprintf(buff, "%lld", value);
#else
  std::sprintf(buff, "%ld", value);
#endif
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(buff, std::strlen(buff));
  value_helper<v8::String>(v8_value);

  // If we decide to use BigInt
  //v8::MaybeLocal<v8::BigInt> v8_value = v8::BigInt::New(v8::Isolate::GetCurrent(), static_cast<int64_t>(value));
  //value_helper<v8::BigInt>(v8_value);
}

void NodeValueWriter::write_uint64(ACE_CDR::ULongLong value)
{
  if (value <= static_cast<ACE_CDR::ULongLong>(NODE_MAX_SAFE_INT)) {
    primitive_helper<ACE_CDR::ULongLong, v8::Number>(value);
    return;
  }

  // If we decide not to use BigInt
  char buff[21]; // 2^64 is 20 characters long in decimal representation, plus null
#ifndef ACE_LINUX
  std::sprintf(buff, "%llu", value);
#else
  std::sprintf(buff, "%lu", value);
#endif
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(buff, std::strlen(buff));
  value_helper<v8::String>(v8_value);

  // If we decide to use BigInt
  //v8::MaybeLocal<v8::BigInt> v8_value = v8::BigInt::NewFromUnsigned(v8::Isolate::GetCurrent(), static_cast<uint64_t>(value));
  //value_helper<v8::BigInt>(v8_value);
}

void NodeValueWriter::write_float32(ACE_CDR::Float value)
{
  primitive_helper<ACE_CDR::Float, v8::Number>(value);
}

void NodeValueWriter::write_float64(ACE_CDR::Double value)
{
  primitive_helper<ACE_CDR::Double, v8::Number>(value);
}

void NodeValueWriter::write_float128(ACE_CDR::LongDouble value)
{
#if ACE_SIZEOF_LONG_DOUBLE == 16
  primitive_helper<ACE_CDR::LongDouble, v8::Number>(value);
#else
  primitive_helper<ACE_CDR::LongDouble::NativeImpl, v8::Number>(value);
#endif
}

void NodeValueWriter::write_fixed(const OpenDDS::FaceTypes::Fixed& /*value*/)
{
}

void NodeValueWriter::write_char8(ACE_CDR::Char value)
{
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(&value, 1);
  value_helper<v8::String>(v8_value);
}

void NodeValueWriter::write_char16(ACE_CDR::WChar value)
{
  const uint16_t c = static_cast<uint16_t>(value);
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(&c, 1);
  value_helper<v8::String>(v8_value);
}

void NodeValueWriter::write_string(const ACE_CDR::Char* value, size_t length)
{
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(value, length);
  value_helper<v8::String>(v8_value);
}

void NodeValueWriter::write_wstring(const ACE_CDR::WChar* value, size_t length)
{
  uint16_t* const str = new uint16_t[length];
  for (size_t i = 0; i < length; ++i) {
    str[i] = static_cast<uint16_t>(value[i]);
  }
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(str, length);
  value_helper<v8::String>(v8_value);
  delete [] str;
}

void NodeValueWriter::write_enum(const char* name, ACE_CDR::Long value)
{
  ACE_UNUSED_ARG(value);
  v8::MaybeLocal<v8::String> v8_value = Nan::New<v8::String>(name);
  value_helper<v8::String>(v8_value);
}

} // NodeOpenDDS
