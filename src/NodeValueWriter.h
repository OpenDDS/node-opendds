#ifndef NODE_VALUE_WRITER_H
#define NODE_VALUE_WRITER_H

#if !defined (ACE_LACKS_PRAGMA_ONCE)
#pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include <dds/DCPS/ValueWriter.h>

#include <nan.h>

namespace NodeOpenDDS {

class NodeValueWriter : public OpenDDS::DCPS::ValueWriter {
public:
  NodeValueWriter();

  void use_bigint(bool value);

  bool begin_struct(OpenDDS::DCPS::Extensibility extensibility);
  bool end_struct();
  bool begin_struct_member(OpenDDS::DCPS::MemberParam params);
  bool end_struct_member();

  bool begin_union(OpenDDS::DCPS::Extensibility extensibility);
  bool end_union();
  bool begin_discriminator(OpenDDS::DCPS::MemberParam params);
  bool end_discriminator();
  bool begin_union_member(OpenDDS::DCPS::MemberParam params);
  bool end_union_member();

  bool begin_array(OpenDDS::XTypes::TypeKind elem_kind);
  bool end_array();
  bool begin_sequence(OpenDDS::XTypes::TypeKind elem_kind, ACE_CDR::ULong length);
  bool end_sequence();
  bool begin_element(ACE_CDR::ULong idx);
  bool end_element();

  bool write_boolean(ACE_CDR::Boolean value);
  bool write_byte(ACE_CDR::Octet value);
#if OPENDDS_HAS_EXPLICIT_INTS
  bool write_int8(ACE_CDR::Int8 value);
  bool write_uint8(ACE_CDR::UInt8 value);
#endif
  bool write_int16(ACE_CDR::Short value);
  bool write_uint16(ACE_CDR::UShort value);
  bool write_int32(ACE_CDR::Long value);
  bool write_uint32(ACE_CDR::ULong value);
  bool write_int64(ACE_CDR::LongLong value);
  bool write_uint64(ACE_CDR::ULongLong value);
  bool write_float32(ACE_CDR::Float value);
  bool write_float64(ACE_CDR::Double value);
  bool write_float128(ACE_CDR::LongDouble value);
  bool write_fixed(const ACE_CDR::Fixed& value);
  bool write_char8(ACE_CDR::Char value);
  bool write_char16(ACE_CDR::WChar value);
  bool write_string(const ACE_CDR::Char* value, size_t length);
  bool write_wstring(const ACE_CDR::WChar* value, size_t length);
  bool write_enum(ACE_CDR::Long value, const OpenDDS::DCPS::EnumHelper& helper);
  bool write_bitmask(ACE_CDR::ULongLong value, const OpenDDS::DCPS::BitmaskHelper& helper);
  bool write_absent_value();

  template <typename V>
  void value_helper(v8::MaybeLocal<V>& v8_value)
  {
    if (v8_value.IsEmpty()) {
      return;
    }

    if (object_stack_.empty()) {
      return;
    }

    if (object_stack_.back()->IsArray()) {
      uint32_t next_index = static_cast<uint32_t>(next_index_);
      Nan::Set(object_stack_.back(), next_index, v8_value.ToLocalChecked());
    } else {
      v8::MaybeLocal<v8::String> key = Nan::New<v8::String>(next_key_.c_str());
      if (!key.IsEmpty()) {
        Nan::Set(object_stack_.back(), key.ToLocalChecked(), v8_value.ToLocalChecked());
      }
    }
  }

  template <typename T, typename V>
  void primitive_helper(T value)
  {
    v8::MaybeLocal<V> v8_value = Nan::New<V>(value);
    value_helper<V>(v8_value);
  }

  template <typename T>
  void object_helper()
  {
    v8::MaybeLocal<v8::Object> temp = Nan::New<T>();
    if (temp.IsEmpty()) {
      return;
    }

    if (!object_stack_.empty()) {
      if (object_stack_.back()->IsArray()) {
        uint32_t next_index = static_cast<uint32_t>(next_index_);
        Nan::Set(object_stack_.back(), next_index, temp.ToLocalChecked());
      } else {
        v8::MaybeLocal<v8::String> key = Nan::New<v8::String>(next_key_.c_str());
        if (!key.IsEmpty()) {
          Nan::Set(object_stack_.back(), key.ToLocalChecked(), temp.ToLocalChecked());
        }
      }
    }
    object_stack_.push_back(temp.ToLocalChecked());
  }

  v8::Local<v8::Object> get_result() { v8::Local<v8::Object> result = result_; result_.Clear(); return result; }

private:

  v8::Local<v8::Object> result_;
  OPENDDS_VECTOR(v8::Local<v8::Object>) object_stack_;
  std::string next_key_;
  size_t next_index_;
  bool use_bigint_;
};

} // NodeOpenDDS

#endif  /* NODE_VALUE_WRITER_H */
