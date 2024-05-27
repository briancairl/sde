/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture.hpp
 */
#pragma once

// C++ Standard Library
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

  decltype(auto) operator[](std::size_t index) { return slots[index]; }
  decltype(auto) operator[](std::size_t index) const { return slots[index]; }

  decltype(auto) begin() { return slots.begin(); }
  decltype(auto) end() { return slots.end(); }

  decltype(auto) begin() const { return slots.begin(); }
  decltype(auto) end() const { return slots.end(); }
};

std::ostream& operator<<(std::ostream& os, const TextureUnits& tu);

}  // namespace sde::graphics
