/**
 * @copyright 2024-present Brian Cairl
 *
 * @file image_ref.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/typecode.hpp"

namespace sde::graphics
{

/**
 * @brief Image channel specifier
 */
enum class ImageChannels : std::uint8_t
{
  kDefault,
  kGrey,
  kGreyA,
  kRGB,
  kRGBA
};

/**
 * @brief Image dimensions
 */
struct ImageRef
{
  /// Pixel format
  ImageChannels channels = ImageChannels::kRGB;
  /// Pixel but depth
  TypeCode element_type = TypeCode::kUInt8;
  /// Image width (columns)
  int width = 0;
  /// Image height (rows)
  int height = 0;
  /// Image data (in memory)
  void* data = nullptr;

  constexpr bool isValid() const { return data != nullptr; }

  constexpr std::size_t pixels() const { return static_cast<std::size_t>(width * height); }
};

}  // namespace sde::graphics
