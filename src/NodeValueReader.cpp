#include "NodeValueReader.h"

#include <string>
#include <iostream>

namespace NodeOpenDDS {

namespace {

template <typename T>
std::string json_stringify(v8::Local<T> input)
{
  Nan::JSON NanJSON;
  v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(input);
  std::string result;
  if (!obj.IsEmpty()) {
    v8::Local<v8::String> strl = NanJSON.Stringify(obj).ToLocalChecked();
    result = std::string(strl->Utf8Length(v8::Isolate::GetCurrent()), 0);
    strl->WriteUtf8(v8::Isolate::GetCurrent(), &result[0]);
  }
  return result;
}

} // namespace

NodeValueReader::NodeValueReader(v8::Local<v8::Object> obj)
{
  indent_ = 0;
  //std::cout << indent(0) << "NodeValueReader::NodeValueReader() - " << json_stringify(obj) << std::endl;

  // Nest initial object so first begin_struct behaves appropriately
  current_object_ = Nan::New<v8::Object>();
  Nan::Set(current_object_, 0, obj);

  current_index_ = 0;
  use_name_ = false;
}

std::string NodeValueReader::indent(int mod)
{
  // If decreasing indent, adjust before generating indent string
  if (mod < 0) {
    indent_ += mod;
  }

  std::string result(indent_, ' ');

  // If increasing indent, adjust after generating indent string
  if (mod > 0) {
    indent_ += mod;
  }

  return result;
}

bool NodeValueReader::begin_struct()
{
  //std::cout << indent(1) << "NodeValueReader::begin_struct()" << std::endl;
  object_stack_.push_back(current_object_);
  index_stack_.push_back(current_index_);

  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    if (mlvai.ToLocalChecked()->IsObject()) {
      current_object_ = v8::Local<v8::Object>::Cast(mlvai.ToLocalChecked());
      current_index_ = 0;
      use_name_ = !current_object_->IsArray();
      property_names_ = Nan::GetPropertyNames(current_object_);
      current_property_name_.Clear();
      return true;
    }
  }
  return false;
}

bool NodeValueReader::end_struct()
{
  //std::cout << indent(-1) << "NodeValueReader::end_struct()" << std::endl;
  current_object_ = object_stack_.back();
  object_stack_.pop_back();
  current_index_ = index_stack_.back();
  index_stack_.pop_back();
  use_name_ = !current_object_->IsArray();

  property_names_ = Nan::GetPropertyNames(current_object_);
  current_property_name_.Clear();

  return true;
}

bool NodeValueReader::begin_struct_member(OpenDDS::XTypes::MemberId& member_id, const OpenDDS::DCPS::MemberHelper& helper)
{
  //std::cout << indent(1) << "NodeValueReader::begin_struct_member()" << std::endl;
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
            //std::cout << indent(0) << " - found member '" << str << "' with member_id of '" << member_id << "'" << std::endl;
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
  //std::cout << indent(-1) << "NodeValueReader::end_struct_member()" << std::endl;
  ++current_index_;
  return true;
}

bool NodeValueReader::begin_union()
{
  //std::cout << indent(1) << "NodeValueReader::begin_union()" << std::endl;
  object_stack_.push_back(current_object_);
  index_stack_.push_back(current_index_);

  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty() && mlvai.ToLocalChecked()->IsObject()) {
    current_object_ = v8::Local<v8::Object>::Cast(mlvai.ToLocalChecked());
    current_index_ = 0;
    use_name_ = !current_object_->IsArray();
    property_names_ = Nan::GetPropertyNames(current_object_);
    current_property_name_.Clear();
    return true;
  }
  return false;
}

bool NodeValueReader::end_union()
{
  //std::cout << indent(-1) << "NodeValueReader::end_union()" << std::endl;
  current_object_ = object_stack_.back();
  object_stack_.pop_back();
  current_index_ = index_stack_.back();
  index_stack_.pop_back();
  use_name_ = !current_object_->IsArray();

  property_names_ = Nan::GetPropertyNames(current_object_);
  current_property_name_.Clear();

  return true;
}

bool NodeValueReader::begin_discriminator()
{
  //std::cout << indent(1) << "NodeValueReader::begin_descriminator()" << std::endl;
  if (!property_names_.IsEmpty()) {
    while (property_names_.ToLocalChecked()->Length() > current_index_) {
      Nan::MaybeLocal<v8::Value> mlv = Nan::Get(property_names_.ToLocalChecked(), current_index_);
      if (!mlv.IsEmpty()) {
        current_property_name_ = mlv.ToLocalChecked();
        v8::Local<v8::String> name = v8::Local<v8::String>::Cast(current_property_name_);
        if (!name.IsEmpty()) {
          std::string str(name->Utf8Length(v8::Isolate::GetCurrent()), 0);
          name->WriteUtf8(v8::Isolate::GetCurrent(), &str[0]);
          //std::cout << indent(0) << " - found property with name '" << str << "'" << std::endl;
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
  //std::cout << indent(-1) << "NodeValueReader::end_descriminator()" << std::endl;
  current_index_ = 0;
  return true;
}

bool NodeValueReader::begin_union_member()
{
  //std::cout << indent(1) << "NodeValueReader::begin_union_member()" << std::endl;
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
  //std::cout << indent(-1) << "NodeValueReader::end_union_member()" << std::endl;
  current_index_ = 0;
  return true;
}

bool NodeValueReader::begin_array()
{
  //std::cout << indent(1) << "NodeValueReader::begin_array()" << std::endl;
  object_stack_.push_back(current_object_);
  index_stack_.push_back(current_index_);

  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty() && mlvai.ToLocalChecked()->IsObject()) {
    current_object_ = v8::Local<v8::Object>::Cast(mlvai.ToLocalChecked());
    current_index_ = 0;
    use_name_ = !current_object_->IsArray();
    property_names_ = Nan::GetPropertyNames(current_object_);
    current_property_name_.Clear();
    return true;
  }
  return false;
}

bool NodeValueReader::end_array()
{
  //std::cout << indent(-1) << "NodeValueReader::end_array()" << std::endl;
  current_object_ = object_stack_.back();
  object_stack_.pop_back();
  current_index_ = index_stack_.back();
  index_stack_.pop_back();
  use_name_ = !current_object_->IsArray();

  property_names_ = Nan::GetPropertyNames(current_object_);
  current_property_name_.Clear();

  return true;
}

bool NodeValueReader::begin_sequence()
{
  //std::cout << indent(1) << "NodeValueReader::begin_sequence() current_property_name_.IsEmpty() = " << current_property_name_.IsEmpty() << ", current_index_ = " << current_index_ << std::endl;
  object_stack_.push_back(current_object_);
  index_stack_.push_back(current_index_);

  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty() && mlvai.ToLocalChecked()->IsObject()) {
    current_object_ = v8::Local<v8::Object>::Cast(mlvai.ToLocalChecked());
    current_index_ = 0;
    use_name_ = !current_object_->IsArray();
    property_names_ = Nan::GetPropertyNames(current_object_);
    current_property_name_.Clear();
    //std::cout << "current_object_ == " << json_stringify(current_object_) << std::endl;
    return true;
  }
  return false;
}

bool NodeValueReader::elements_remaining()
{
  //std::cout << indent(0) << "NodeValueReader::elements_remaining()" << std::endl;
  v8::Local<v8::Array> array = v8::Local<v8::Array>::Cast(current_object_);
  if (!array.IsEmpty()) {
    return array->Length() > current_index_;
  }
  return false;
}

bool NodeValueReader::end_sequence()
{
  //std::cout << indent(-1) << "NodeValueReader::end_sequence()" << std::endl;
  current_object_ = object_stack_.back();
  object_stack_.pop_back();
  current_index_ = index_stack_.back();
  index_stack_.pop_back();
  use_name_ = !current_object_->IsArray();

  property_names_ = Nan::GetPropertyNames(current_object_);
  current_property_name_.Clear();

  return true;
}

bool NodeValueReader::begin_element()
{
  //std::cout << indent(1) << "NodeValueReader::begin_element()" << std::endl;
  return true;
}

bool NodeValueReader::end_element()
{
  //std::cout << indent(-1) << "NodeValueReader::end_element()" << std::endl;
  ++current_index_;
  return true;
}

bool NodeValueReader::read_boolean(ACE_CDR::Boolean& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_boolean()" << std::endl;
  return primitive_helper<v8::Boolean>(value, &v8::Value::IsBoolean);
}

bool NodeValueReader::read_byte(ACE_CDR::Octet& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_byte()" << std::endl;
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

#if OPENDDS_HAS_EXPLICIT_INTS

bool NodeValueReader::read_int8(ACE_CDR::Int8& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_int8()" << std::endl;
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
}

bool NodeValueReader::read_uint8(ACE_CDR::UInt8& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_uint8()" << std::endl;
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

#endif

bool NodeValueReader::read_int16(ACE_CDR::Short& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_int16()" << std::endl;
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
}

bool NodeValueReader::read_uint16(ACE_CDR::UShort& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_uint16()" << std::endl;
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

bool NodeValueReader::read_int32(ACE_CDR::Long& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_int32()" << std::endl;
  bool result = primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
  if (result) {
    //std::cout << indent(0) << " - found value '" << value << "'" << std::endl;
  }
  return result;
}

bool NodeValueReader::read_uint32(ACE_CDR::ULong& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_uint32()" << std::endl;
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

bool NodeValueReader::read_int64(ACE_CDR::LongLong& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_int64()" << std::endl;
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
#ifndef ACE_LINUX
        int result = std::sscanf(temp.c_str(), "%lld", &value);
#else
        int result = std::sscanf(temp.c_str(), "%ld", &value);
#endif
        return result != 0;
      }
    }
  }
  return false;
}

bool NodeValueReader::read_uint64(ACE_CDR::ULongLong& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_uint64()" << std::endl;
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
#ifndef ACE_LINUX
        int result = std::sscanf(temp.c_str(), "%llu", &value);
#else
        int result = std::sscanf(temp.c_str(), "%lu", &value);
#endif
        return result != 0;
      }
    }
  }
  return false;
}

bool NodeValueReader::read_float32(ACE_CDR::Float& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_float32()" << std::endl;
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtof);
}

bool NodeValueReader::read_float64(ACE_CDR::Double& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_float64()" << std::endl;
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtod);
}

bool NodeValueReader::read_float128(ACE_CDR::LongDouble& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_float128(LongDouble&)" << std::endl;
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtold);
}

#ifdef NONNATIVE_LONGDOUBLE

bool NodeValueReader::read_float128(long double& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_float128() (long double&)" << std::endl;
  return primitive_helper<v8::Number>(value, &v8::Value::IsNumber, strtold);
}

#endif

bool NodeValueReader::read_fixed(OpenDDS::FaceTypes::Fixed& /*value*/)
{
  //std::cout << indent(0) << "NodeValueReader::read_fixed" << std::endl;
  return false;
}

bool NodeValueReader::read_char8(ACE_CDR::Char& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_char8" << std::endl;
  std::string temp;
  if (read_string(temp)) {
    value = temp[0];
    return true;
  }
  return false;
}

bool NodeValueReader::read_char16(ACE_CDR::WChar& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_char16" << std::endl;
  std::wstring temp;
  if (read_wstring(temp)) {
    value = temp[0];
    return true;
  }
  return false;
}

bool NodeValueReader::read_string(std::string& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_string()" << std::endl;
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    Nan::MaybeLocal<v8::String> tov = Nan::To<v8::String>(mlvai.ToLocalChecked());
    if (!tov.IsEmpty()) {
      value = std::string(tov.ToLocalChecked()->Utf8Length(v8::Isolate::GetCurrent()), 0);
      tov.ToLocalChecked()->WriteUtf8(v8::Isolate::GetCurrent(), &value[0]);
      //std::cout << indent(0) << " - found value '" << value << "'" << std::endl;
      return true;
    }
  }
  return false;
}

bool NodeValueReader::read_wstring(std::wstring& value)
{
  //std::cout << indent(0) << "NodeValueReader::read_wstring()" << std::endl;
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    Nan::MaybeLocal<v8::String> tov = Nan::To<v8::String>(mlvai.ToLocalChecked());
    if (!tov.IsEmpty()) {
      std::vector<uint16_t> temp(tov.ToLocalChecked()->Length(), 0);
      tov.ToLocalChecked()->Write(v8::Isolate::GetCurrent(), &temp[0]);
      value = std::wstring(temp.size(), 0);
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
  //std::cout << indent(0) << "NodeValueReader::read_long_enum()" << std::endl;
  if (primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol)) {
    //std::cout << indent(0) << " - found enum with value '" << value << "'" << std::endl;
  }

  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
    {
      std::string temp;
      if (read_string(temp)) {
        //std::cout << indent(0) << " - found enum with string '" << temp << "'" << std::endl;
        bool result = helper.get_value(value, temp.c_str());
        //std::cout << indent(0) << " - found enum with string '" << temp << "'" << " returning " << result << " with value " << value << std::endl;
        return result;
      }
    }
  }

  return false;
}

} // namespace NodeOpenDDS

OPENDDS_END_VERSIONED_NAMESPACE_DECL
