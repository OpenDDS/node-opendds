#include "NodeValueReader.h"

#include <codecvt>
#include <locale>

#if NODE_MAJOR_VERSION > 10 || (NODE_MAJOR_VERSION == 10 && NODE_MINOR_VERSION >= 4)
#define HAS_BIGINT
#endif

#ifdef HAS_BIGINT
// This is a bit of a hack to make Nan::To work for BigInt
// It should be OK as long as we avoid trying to reference BigInt before 10.4
namespace Nan {
namespace imp {

template<>
struct ToFactory<v8::BigInt> : ToFactoryBase<v8::BigInt> {
  static inline return_t convert(v8::Local<v8::Value> val) {
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::EscapableHandleScope scope(isolate);
    return scope.Escape(val->ToBigInt(isolate->GetCurrentContext()).FromMaybe(v8::Local<v8::BigInt>()));
  }
};

}
}
#endif

namespace NodeOpenDDS {

NodeValueReader::NodeValueReader(v8::Local<v8::Object> obj)
{
  // Nest initial object so first begin_struct behaves appropriately
  current_object_ = Nan::New<v8::Object>();
  Nan::Set(current_object_, 0, obj);

  current_index_ = 0;
  use_name_ = false;
}

bool NodeValueReader::begin_struct(OpenDDS::DCPS::Extensibility /*extensibility*/)
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
        Nan::Utf8String name(current_property_name_);
        if (helper.get_value(member_id, *name)) {
          return true;
        }
      }
      ++current_index_;
    }
  }
  return false;
}

bool NodeValueReader::members_remaining()
{
  // TODO(sonndinh):
  return true;
}

bool NodeValueReader::end_struct_member()
{
  ++current_index_;
  return true;
}

bool NodeValueReader::begin_union(OpenDDS::DCPS::Extensibility /*extensibility*/)
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
        Nan::Utf8String name(current_property_name_);
        if (std::strcmp(*name, "$discriminator") == 0) {
          return true;
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
        Nan::Utf8String name(current_property_name_);
        if (std::strcmp(*name, "$discriminator") != 0) {
          return true;
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

bool NodeValueReader::begin_array(OpenDDS::XTypes::TypeKind /*elem_kind*/)
{
  return begin_nested();
}

bool NodeValueReader::end_array()
{
  return end_nested();
}

bool NodeValueReader::begin_sequence(OpenDDS::XTypes::TypeKind /*elem_kind*/)
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
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtol);
}

bool NodeValueReader::read_uint32(ACE_CDR::ULong& value)
{
  return primitive_helper<v8::Integer>(value, &v8::Value::IsNumber, strtoul);
}

bool NodeValueReader::read_int64(ACE_CDR::LongLong& value)
{
  Nan::MaybeLocal<v8::Value> mlvai = use_name_ ? Nan::Get(current_object_, current_property_name_) : Nan::Get(current_object_, current_index_);
  if (!mlvai.IsEmpty()) {
#ifdef HAS_BIGINT
    if (!mlvai.ToLocalChecked()->IsNumber()) {
      Nan::MaybeLocal<v8::BigInt> tov = Nan::To<v8::BigInt>(mlvai.ToLocalChecked());
      if (!tov.IsEmpty()) {
        value = static_cast<ACE_CDR::LongLong>(tov.ToLocalChecked()->Int64Value());
        return true;
      }
    }
#endif
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
#ifdef HAS_BIGINT
    if (!mlvai.ToLocalChecked()->IsNumber()) {
      Nan::MaybeLocal<v8::BigInt> tov = Nan::To<v8::BigInt>(mlvai.ToLocalChecked());
      if (!tov.IsEmpty()) {
        value = static_cast<ACE_CDR::LongLong>(tov.ToLocalChecked()->Uint64Value());
        return true;
      }
    }
#endif
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
      Nan::Utf8String str(tov.ToLocalChecked());
      value = *str;
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
      Nan::Utf8String str(tov.ToLocalChecked());
      std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wconv;
      value = wconv.from_bytes(*str);
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

bool NodeValueReader::read_bitmask(ACE_CDR::ULongLong& value, const OpenDDS::DCPS::BitmaskHelper& helper)
{
  // TODO(sonndinh):
  return true;
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
