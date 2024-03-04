// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/typecode.hpp"

namespace sde::graphics
{
std::ostream& operator<<(std::ostream& os, TypeCode channels)
{
  switch (channels)
  {
  case TypeCode::kFloat32:
    return os << "Float32";
  case TypeCode::kFloat64:
    return os << "Float64";
  case TypeCode::kSInt32:
    return os << "SInt32";
  case TypeCode::kUInt32:
    return os << "UInt32";
  case TypeCode::kSInt16:
    return os << "SInt16";
  case TypeCode::kUInt16:
    return os << "UInt16";
  case TypeCode::kSInt8:
    return os << "SInt8";
  case TypeCode::kUInt8:
    return os << "UInt8";
  }
  return os;
}
}  // namespace sde::graphics