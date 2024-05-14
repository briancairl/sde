/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture_handle.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>

namespace sde::graphics
{

struct TextureHandle
{
  std::size_t id = 0;
  static constexpr TextureHandle null() { return {0}; }
};

inline bool operator<(TextureHandle lhs, TextureHandle rhs) { return lhs.id < rhs.id; }
inline bool operator>(TextureHandle lhs, TextureHandle rhs) { return lhs.id > rhs.id; }
inline bool operator==(TextureHandle lhs, TextureHandle rhs) { return lhs.id == rhs.id; }
inline bool operator!=(TextureHandle lhs, TextureHandle rhs) { return lhs.id != rhs.id; }

struct TextureHandleHash
{
  constexpr std::size_t operator()(const TextureHandle& handle) const { return handle.id; }
};

inline std::ostream& operator<<(std::ostream& os, TextureHandle handle) { return os << "{ id: " << handle.id << " }"; }

}  // namespace sde::graphics


namespace std
{

template <> struct hash<sde::graphics::TextureHandle> : sde::graphics::TextureHandleHash
{};

}  // namespace std