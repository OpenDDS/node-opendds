#include "NodeValueReader.h"

namespace NodeOpenDDS {

NodeValueReader::NodeValueReader(v8::Local<v8::Object> obj)
{
  // Nest initial object so first begin_struct behaves appropriately
  current_object_ = Nan::New<v8::Object>();
  Nan::Set(current_object_, 0, obj);

  current_index_ = 0;
  use_name_ = false;
}

bool NodeValueReader::begin_struct()
{
  return begin_nested();
}

bool NodeValueReader::end_struct()
{
  return end_nested();
}

bool NodeValueReader::begin_struct_member(OpenDDS::XTypes::MemberId& member_id, const OpenDDS::DCPS::MemberHelper& helper)
{
  if (!property_names_.IsEmpty()) {
    while (property_names_.ToLocalChecked()->Length() > current_index_) {
      Nan::MaybeLocal<v8::Value> mlv = Nan::Get(property_names_.ToLocalChecked(), current_index_);
      if (!mlv.IsEmpty()) {
        current_property_name_ = mlv.ToLocalChecked();
        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(current_property_name_);
        if (!name.IsEmpty()) {
          std::string str(name->Utf8Length(v8::Isolate::GetCurrent()), 0);
          name->WriteUtf8(v8::Isolate::GetCurrent(), &str[0]);
          if (helper.get_value(member_id, str.c_str())) {
            return true;
          }
        }
      }
      ++current_index_;
    }
  }
  return false;
}

bool NodeValueReader::end_struct_member()
{
  ++current_index_;
  return true;
}

bool NodeValueReader::begin_union()
{
  return begin_nested();
}

bool NodeValueReader::end_union()
{
  return end_nested();
}

bool NodeValueReader::begin_discriminator()
{
  if (!property_names_.IsEmpty()) {
    while (property_names_.ToLocalChecked()->Length() > current_index_) {
      Nan::MaybeLocal<v8::Value> mlv = Nan::Get(property_names_.ToLocalChecked(), current_index_);
      if (!mlv.IsEmpty()) {
        current_property_name_ = mlv.ToLocalChecked();
        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(current_property_name_);
        if (!name.IsEmpty()) {
          std::string str(name->Utf8Length(v8::Isolate::GetCurrent()), 0);
          name->WriteUtf8(v8::Isolate::GetCurrent(), &str[0]);
          if (str == "$discriminator") {
            return true;
          }
        }
      }
      ++current_index_;
    }
  }
  return false;
}

bool NodeValueReader::end_discriminator()
{
  current_index_ = 0;
  return true;
}

bool NodeValueReader::begin_union_member()
{
  if (!property_names_.IsEmpty()) {
    while (property_names_.ToLocalChecked()->Length() > current_index_) {
      Nan::MaybeLocal<v8::Value> mlv = Nan::Get(property_names_.ToLocalChecked(), current_index_);
      if (!mlv.IsEmpty()) {
        current_property_name_ = mlv.ToLocalChecked();
        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(current_property_name_);
        if (!name.IsEmpty()) {
          std::string str(name->Utf8Length(v8::Isolate::GetCurrent()), 0);
          name->WriteUtf8(v8::Isolate::GetCurrent(), &str[0]);
          if (str != "$discriminator") {
            return true;
          }
        }
      }
      ++current_index_;
    }
  }
  return false;
}

bool NodeValueReader::end_union_member()
{
  current_index_ = 0;
  return true;
}

bool NodeValueReader::begin_array()
{
  return begin_nested();
}

bool NodeValueReader::end_array()
{
  return end_nested();
}

bool NodeValueReader::begin_sequence()
{
  return begin_nested();
}

bool NodeValueReader::elements_remaining()
{
  v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(current_object_);
  if (!array.IsEmpty()) {
    return array->Length() > current_index_;
  }
  return false;
}

bool NodeValueReader::end_sequence()
{
  return end_nested();
}

bool NodeValueReader::begin_element()
{
  return true;
}

bool NodeValueReader::end_element()
{
  ++current_index_;
  return true;
}

bool NodeValueReader::read_boolean(ACE_CDR::Boolean& value)
{
  return primitive_helper<v8::Boolean>(value, &v8::Value::IsBoolean);
}

bool NodeValueReader::read_byte(ACE_CDR::Octet& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

#if OPENDDS_HAS_EXPLICIT_INTS

bool NodeValueReader::read_int8(ACE_CDR::Int8& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
}

bool NodeValueReader::read_uint8(ACE_CDR::UInt8& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

#endif

bool NodeValueReader::read_int16(ACE_CDR::Short& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
}

bool NodeValueReader::read_uint16(ACE_CDR::UShort& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

bool NodeValueReader::read_int32(ACE_CDR::Long& value)
{
  bool result = primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
  if (result) {
  }
  return result;
}

bool NodeValueReader::read_uint32(ACE_CDR::ULong& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

bool NodeValueReader::read_int64(ACE_CDR::LongLong& value)
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    // This doesn't work because NaN seems to lack conversion support to BigInt
    // Not super surprising, since BigInt support wasn't introduced until Node.js 10.4
    //{
    //  Nan::MaybeLocal<v8::BigInt> tov = Nan::To<v8::BigInt>(mlvai.ToLocalChecked());
    //  if (!tov.IsEmpty()) {
    //    value = static_cast<ACE_CDR::LongLong>(tov.ToLocalChecked()->Int64Value());
    //    return true;
    //  }
    //}
    {
      Nan::MaybeLocal<v8::String> tov = Nan::To<v8::String>(mlvai.ToLocalChecked());
      if (!tov.IsEmpty()) {
        std::string temp;
        read_string(temp);
        int result = std::sscanf(temp.c_str(), ACE_INT64_FORMAT_SPECIFIER_ASCII, &value);
        return result != 0;
      }
    }
  }
  return false;
}

bool NodeValueReader::read_uint64(ACE_CDR::ULongLong& value)
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    // This doesn't work because NaN seems to lack conversion support to BigInt
    // Not super surprising, since BigInt support wasn't introduced until Node.js 10.4
    //{
    //  Nan::MaybeLocal<v8::BigInt> tov = Nan::To<v8::BigInt>(mlvai.ToLocalChecked());
    //  if (!tov.IsEmpty()) {
    //    value = static_cast<ACE_CDR::LongLong>(tov.ToLocalChecked()->UInt64Value());
    //    return true;
    //  }
    //}
    {
      Nan::MaybeLocal<v8::String> tov = Nan::To<v8::String>(mlvai.ToLocalChecked());
      if (!tov.IsEmpty()) {
        std::string temp;
        read_string(temp);
        int result = std::sscanf(temp.c_str(), ACE_UINT64_FORMAT_SPECIFIER_ASCII, &value);
        return result != 0;
      }
    }
  }
  return false;
}

bool NodeValueReader::read_float32(ACE_CDR::Float& value)
{
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtof);
}

bool NodeValueReader::read_float64(ACE_CDR::Double& value)
{
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtod);
}

bool NodeValueReader::read_float128(ACE_CDR::LongDouble& value)
{
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtold);
}

#ifdef NONNATIVE_LONGDOUBLE

bool NodeValueReader::read_float128(long double& value)
{
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtold);
}

#endif

bool NodeValueReader::read_fixed(OpenDDS::FaceTypes::Fixed& /*value*/)
{
  return false;
}

bool NodeValueReader::read_char8(ACE_CDR::Char& value)
{
  std::string temp;
  if (read_string(temp)) {
    value = temp[0];
    return true;
  }
  return false;
}

bool NodeValueReader::read_char16(ACE_CDR::WChar& value)
{
  std::wstring temp;
  if (read_wstring(temp)) {
    value = temp[0];
    return true;
  }
  return false;
}

bool NodeValueReader::read_string(std::string& value)
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    Nan::MaybeLocal<v8::String> tov = Nan::To<v8::String>(mlvai.ToLocalChecked());
    if (!tov.IsEmpty()) {
      value.resize(tov.ToLocalChecked()->Utf8Length(v8::Isolate::GetCurrent()), 0);
      tov.ToLocalChecked()->WriteUtf8(v8::Isolate::GetCurrent(), &value[0], -1, 0, v8::String::NO_NULL_TERMINATION);
      return true;
    }
  }
  return false;
}

bool NodeValueReader::read_wstring(std::wstring& value)
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    Nan::MaybeLocal<v8::String> tov = Nan::To<v8::String>(mlvai.ToLocalChecked());
    if (!tov.IsEmpty()) {
      std::vector<uint16_t> temp(tov.ToLocalChecked()->Length(), 0);
      tov.ToLocalChecked()->Write(v8::Isolate::GetCurrent(), &temp[0], 0, -1, v8::String::NO_NULL_TERMINATION);
      value.resize(temp.size(), 0);
      for (size_t i = 0; i < temp.size(); ++i) {
        value[i] = static_cast<wchar_t>(temp[i]);
      }
      return true;
    }
  }
  return false;
}

bool NodeValueReader::read_long_enum(ACE_CDR::Long& value, const OpenDDS::DCPS::EnumHelper& helper)
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    std::string temp;
    if (read_string(temp)) {
      bool result = helper.get_value(value, temp.c_str());
      return result;
    }
  }

  if (primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol)) {
    return true;
  }

  return false;
}

bool NodeValueReader::begin_nested()
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty() && mlvai.ToLocalChecked()->IsObject()) {
    object_stack_.push_back(current_object_);
    index_stack_.push_back(current_index_);

    current_object_ = v8::Local<v8::Object>::Cast(mlvai.ToLocalChecked());
    current_index_ = 0;
    use_name_ = !current_object_->IsArray();

    property_names_ = Nan::GetPropertyNames(current_object_);
    current_property_name_.Clear();

    return true;
  }
  return false;
}

bool NodeValueReader::end_nested()
{
  current_object_ = object_stack_.back();
  object_stack_.pop_back();
  current_index_ = index_stack_.back();
  index_stack_.pop_back();
  use_name_ = !current_object_->IsArray();

  property_names_ = Nan::GetPropertyNames(current_object_);
  current_property_name_.Clear();

  return true;
}

} // namespace NodeOpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
