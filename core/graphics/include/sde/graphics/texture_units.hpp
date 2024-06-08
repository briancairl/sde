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
#include <iterator>
#include <optional>

// SDE
#include "sde/graphics/texture_handle.hpp"

namespace sde::graphics
{

struct TextureUnits
{
public:
  static constexpr std::size_t kAvailable = 16;

  TextureUnits() { reset(); }

  void reset() { slots_.fill(TextureHandle::null()); }

  TextureHandle& operator[](std::size_t index) { return slots_[index]; }
  TextureHandle operator[](std::size_t index) const { return slots_[index]; }

  decltype(auto) begin() { return slots_.begin(); }
  decltype(auto) end() { return slots_.end(); }

  decltype(auto) begin() const { return slots_.begin(); }
  decltype(auto) end() const { return slots_.end(); }

  [[nodiscard]] auto find(TextureHandle handle) const
  {
    return std::find(TextureUnits::begin(), TextureUnits::end(), handle);
  }

  [[nodiscard]] std::optional<std::size_t> get(TextureHandle handle) const
  {
    const auto u_itr = find(handle);
    return (u_itr == end()) ? std::optional<std::size_t>{}
                            : std::make_optional(static_cast<std::size_t>(std::distance(begin(), u_itr)));
  }

  [[nodiscard]] std::optional<std::size_t> operator()(TextureHandle handle) const { return get(handle); }

private:
  std::array<TextureHandle, kAvailable> slots_;
};

std::ostream& operator<<(std::ostream& os, const TextureUnits& tu);

}  // namespace sde::graphics
