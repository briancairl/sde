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

using id_type = std::size_t;

template <typename T> struct ResourceHandle
{
public:
  explicit ResourceHandle(id_type id) : id_{id} {}
  ResourceHandle() : ResourceHandle{0} {}

  constexpr id_type id() const { return id_; }

  constexpr bool is_null() const { return id_ == 0UL; }

  constexpr bool is_valid() const { return id_ != 0UL; }

  constexpr operator bool() const { return is_valid(); }

  static constexpr T null() { return T{0}; }

private:
  id_type id_ = 0;
};

template <typename T> constexpr bool operator<(const ResourceHandle<T>& lhs, const ResourceHandle<T>& rhs)
{
  return lhs.id() < rhs.id();
}
template <typename T> constexpr bool operator>(const ResourceHandle<T>& lhs, const ResourceHandle<T>& rhs)
{
  return lhs.id() > rhs.id();
}
template <typename T> constexpr bool operator==(const ResourceHandle<T>& lhs, const ResourceHandle<T>& rhs)
{
  return lhs.id() == rhs.id();
}
template <typename T> constexpr bool operator!=(const ResourceHandle<T>& lhs, const ResourceHandle<T>& rhs)
{
  return lhs.id() != rhs.id();
}

struct ResourceHandleHash
{
  template <typename T> constexpr std::size_t operator()(const ResourceHandle<T>& handle) const { return handle.id(); }
};

template <typename T> inline std::ostream& operator<<(std::ostream& os, const ResourceHandle<T>& handle)
{
  if (handle.is_null())
  {
    return os << "{ id: <NULL> }";
  }
  else
  {
    return os << "{ id: " << handle.id() << " }";
  }
}

}  // namespace sde::graphics
