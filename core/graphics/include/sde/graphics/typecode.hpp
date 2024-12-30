/**
 * @copyright 2024-present Brian Cairl
 *
 * @file typecode.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>

namespace sde::graphics
{

enum class TypeCode
{
  kFloat32,
  kFloat64,
  kSInt32,
  kUInt32,
  kSInt16,
  kUInt16,
  kSInt8,
  kUInt8,
};

std::ostream& operator<<(std::ostream& os, TypeCode channels);

template <TypeCode C> [[nodiscard]] constexpr std::size_t byte_count() { return 0UL; }

template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kSInt8>() { return 1UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kUInt8>() { return 1UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kSInt16>() { return 2UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kUInt16>() { return 2UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kFloat32>() { return 4UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kFloat64>() { return 8UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kSInt32>() { return 4UL; }
template <> [[nodiscard]] constexpr std::size_t byte_count<TypeCode::kUInt32>() { return 4UL; }

constexpr std::size_t byte_count(TypeCode code)
{
  switch (code)
  {
  case TypeCode::kFloat32:
    return byte_count<TypeCode::kFloat32>();
  case TypeCode::kFloat64:
    return byte_count<TypeCode::kFloat64>();
  case TypeCode::kSInt32:
    return byte_count<TypeCode::kSInt32>();
  case TypeCode::kUInt32:
    return byte_count<TypeCode::kUInt32>();
  case TypeCode::kSInt16:
    return byte_count<TypeCode::kSInt16>();
  case TypeCode::kUInt16:
    return byte_count<TypeCode::kUInt16>();
  case TypeCode::kSInt8:
    return byte_count<TypeCode::kSInt8>();
  case TypeCode::kUInt8:
    return byte_count<TypeCode::kUInt8>();
  }
  return 0UL;
}

template <typename T> [[nodiscard]] constexpr TypeCode typecode() { return TypeCode::kUInt8; };

template <> [[nodiscard]] constexpr TypeCode typecode<float>() { return TypeCode::kFloat32; };
template <> [[nodiscard]] constexpr TypeCode typecode<double>() { return TypeCode::kFloat64; };
template <> [[nodiscard]] constexpr TypeCode typecode<std::int32_t>() { return TypeCode::kSInt32; };
template <> [[nodiscard]] constexpr TypeCode typecode<std::uint32_t>() { return TypeCode::kUInt32; };
template <> [[nodiscard]] constexpr TypeCode typecode<std::int16_t>() { return TypeCode::kSInt16; };
template <> [[nodiscard]] constexpr TypeCode typecode<std::uint16_t>() { return TypeCode::kUInt16; };
template <> [[nodiscard]] constexpr TypeCode typecode<std::int8_t>() { return TypeCode::kSInt8; };
template <> [[nodiscard]] constexpr TypeCode typecode<std::uint8_t>() { return TypeCode::kUInt8; };

}  // namespace sde::graphics
