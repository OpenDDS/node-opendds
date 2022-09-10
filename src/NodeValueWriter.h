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

  void begin_struct();
  void end_struct();
  void begin_struct_member(const DDS::MemberDescriptor& descriptor);
  void end_struct_member();

  void begin_union();
  void end_union();
  void begin_discriminator();
  void end_discriminator();
  void begin_union_member(const char* name);
  void end_union_member();

  void begin_array();
  void end_array();
  void begin_sequence();
  void end_sequence();
  void begin_element(size_t idx);
  void end_element();

  void write_boolean(ACE_CDR::Boolean value);
  void write_byte(ACE_CDR::Octet value);
#if OPENDDS_HAS_EXPLICIT_INTS
  void write_int8(ACE_CDR::Int8 value);
  void write_uint8(ACE_CDR::UInt8 value);
#endif
  void write_int16(ACE_CDR::Short value);
  void write_uint16(ACE_CDR::UShort value);
  void write_int32(ACE_CDR::Long value);
  void write_uint32(ACE_CDR::ULong value);
  void write_int64(ACE_CDR::LongLong value);
  void write_uint64(ACE_CDR::ULongLong value);
  void write_float32(ACE_CDR::Float value);
  void write_float64(ACE_CDR::Double value);
  void write_float128(ACE_CDR::LongDouble value);
  void write_fixed(const OpenDDS::FaceTypes::Fixed& value);
  void write_char8(ACE_CDR::Char value);
  void write_char16(ACE_CDR::WChar value);
  void write_string(const ACE_CDR::Char* value, size_t length);
  void write_wstring(const ACE_CDR::WChar* value, size_t length);
  void write_enum(const char* name, ACE_CDR::Long value);

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
};

} // NodeOpenDDS

#endif  /* NODE_VALUE_WRITER_H */
