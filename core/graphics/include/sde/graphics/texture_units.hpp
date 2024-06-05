/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_units.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <array>
#include <iosfwd>

// SDE
#include "sde/graphics/texture_handle.hpp"

namespace sde::graphics
{

struct TextureUnits
{
  static constexpr std::size_t kAvailable = 16;

  std::array<TextureHandle, kAvailable> slots;

  TextureUnits() { reset(); }

  void reset() { slots.fill(TextureHandle::null()); }

  TextureHandle& operator[](std::size_t index) { return slots[index]; }
  TextureHandle operator[](std::size_t index) const { return slots[index]; }

  decltype(auto) begin() { return slots.begin(); }
  decltype(auto) end() { return slots.end(); }

  decltype(auto) begin() const { return slots.begin(); }
  decltype(auto) end() const { return slots.end(); }

  [[nodiscard]] auto find(TextureHandle handle) const
  {
    return std::find(TextureUnits::begin(), TextureUnits::end(), handle);
  }
};

std::ostream& operator<<(std::ostream& os, const TextureUnits& tu);

}  // namespace sde::graphics
