#ifndef NODE_VALUE_READER_H
#define NODE_VALUE_READER_H

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include <dds/DCPS/ValueReader.h>

#include <nan.h>

namespace NodeOpenDDS {

template <typename T, typename U>
void safe_assign(T& out, const U& in)
{
  out = static_cast<T>(in);
}

template <typename U>
void safe_assign(ACE_CDR::LongDouble& out, const U& in)
{
  ACE_CDR_LONG_DOUBLE_ASSIGNMENT(out, in);
}

class NodeValueReader : public OpenDDS::DCPS::ValueReader {
public:
  explicit NodeValueReader(v8::Local<v8::Object> obj);

  bool begin_struct(OpenDDS::DCPS::Extensibility extensibility);
  bool end_struct();
  bool begin_struct_member(OpenDDS::XTypes::MemberId& member_id, const OpenDDS::DCPS::MemberHelper& helper);
  bool members_remaining();
  bool end_struct_member();

  bool begin_union(OpenDDS::DCPS::Extensibility extensibility);
  bool end_union();
  bool begin_discriminator();
  bool end_discriminator();
  bool begin_union_member();
  bool end_union_member();

  bool begin_array(OpenDDS::XTypes::TypeKind elem_kind);
  bool end_array();
  bool begin_sequence(OpenDDS::XTypes::TypeKind elem_kind);
  bool elements_remaining();
  bool end_sequence();
  bool begin_element();
  bool end_element();

  bool read_boolean(ACE_CDR::Boolean& value);
  bool read_byte(ACE_CDR::Octet& value);
#if OPENDDS_HAS_EXPLICIT_INTS
  bool read_int8(ACE_CDR::Int8& value);
  bool read_uint8(ACE_CDR::UInt8& value);
#endif
  bool read_int16(ACE_CDR::Short& value);
  bool read_uint16(ACE_CDR::UShort& value);
  bool read_int32(ACE_CDR::Long& value);
  bool read_uint32(ACE_CDR::ULong& value);
  bool read_int64(ACE_CDR::LongLong& value);
  bool read_uint64(ACE_CDR::ULongLong& value);
  bool read_float32(ACE_CDR::Float& value);
  bool read_float64(ACE_CDR::Double& value);
  bool read_float128(ACE_CDR::LongDouble& value);

#ifdef NONNATIVE_LONGDOUBLE
  bool read_float128(long double& value);
#endif

  bool read_fixed(ACE_CDR::Fixed& value);
  bool read_char8(ACE_CDR::Char& value);
  bool read_char16(ACE_CDR::WChar& value);
  bool read_string(std::string& value);
  bool read_wstring(std::wstring& value);

  bool read_long_enum(ACE_CDR::Long& value, const OpenDDS::DCPS::EnumHelper& helper);
  bool read_bitmask(ACE_CDR::ULongLong& value, const OpenDDS::DCPS::BitmaskHelper& helper);

private:
  NodeValueReader();

  bool begin_nested();
  bool end_nested();

  typedef bool (v8::Value::*ValueChecker)() const;

  template <typename V, typename T>
  bool primitive_helper(T& value, ValueChecker checker)
  {
    Nan::MaybeLocal<v8::Value> mlvai = current_property_name_.IsEmpty() ? Nan::Get(current_object_, current_index_) : Nan::Get(current_object_, current_property_name_);
    if (!mlvai.IsEmpty()) {
      v8::Local<v8::Value> lvai = mlvai.ToLocalChecked();
      if (((*lvai)->*checker)()) {
        v8::Local<V> tov = v8::Local<V>::Cast(lvai);
        if (!tov.IsEmpty()) {
          safe_assign(value, tov->Value());
          return true;
        }
      }
    }
    return false;
  }

  template <typename V, typename T, typename U>
  bool primitive_helper(T& value, ValueChecker checker, U (*str_conv)(const char*, char**, int base))
  {
    Nan::MaybeLocal<v8::Value> mlvai = current_property_name_.IsEmpty() ? Nan::Get(current_object_, current_index_) : Nan::Get(current_object_, current_property_name_);
    if (!mlvai.IsEmpty()) {
      v8::Local<v8::Value> lvai = mlvai.ToLocalChecked();
      if (((*lvai)->*checker)()) {
        v8::Local<V> tov = v8::Local<V>::Cast(lvai);
        if (!tov.IsEmpty()) {
          safe_assign(value, tov->Value());
          return true;
        }
      }
      if (lvai->IsString()) {
        Nan::Utf8String str(lvai);
        safe_assign(value, (*str_conv)(*str, 0, std::string(*str).find("0x") != std::string::npos ? 16 : 10));
        return true;
      }
    }
    return false;
  }

  template <typename V, typename T, typename U>
  bool primitive_helper(T& value, ValueChecker checker, U (*str_conv)(const char*, char**))
  {
    Nan::MaybeLocal<v8::Value> mlvai = current_property_name_.IsEmpty() ? Nan::Get(current_object_, current_index_) : Nan::Get(current_object_, current_property_name_);
    if (!mlvai.IsEmpty()) {
      v8::Local<v8::Value> lvai = mlvai.ToLocalChecked();
      if (((*lvai)->*checker)()) {
        v8::Local<V> tov = v8::Local<V>::Cast(lvai);
        if (!tov.IsEmpty()) {
          safe_assign(value, tov->Value());
          return true;
        }
      }
      if (lvai->IsString()) {
        Nan::Utf8String str(lvai);
        safe_assign(value, (*str_conv)(*str, 0));
        return true;
      }
    }
    return false;
  }

  v8::Local<v8::Object> current_object_;
  uint32_t current_index_;

  v8::MaybeLocal<v8::Array> property_names_;
  v8::Local<v8::Value> current_property_name_;

  OPENDDS_VECTOR(v8::Local<v8::Object>) object_stack_;
  OPENDDS_VECTOR(uint32_t) index_stack_;

  bool use_name_;
};

} // namespace NodeOpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL

#endif  /* NODE_VALUE_READER_H */
