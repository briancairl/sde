/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_handle.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>

namespace sde
{

template <typename T, typename IdentifierT = std::size_t, auto kNullValue = 0> struct ResourceHandle
{
public:
  using id_type = IdentifierT;

  explicit ResourceHandle(IdentifierT id) : id_{id} {}

  ResourceHandle(const ResourceHandle&) = default;

  ResourceHandle(ResourceHandle&& other) : id_{other.id_} { other.id_ = kNullValue; }

  ResourceHandle() : ResourceHandle{kNullValue} {}

  ResourceHandle& operator=(const ResourceHandle&) = default;

  ResourceHandle& operator=(ResourceHandle&& other)
  {
    id_ = other.id_;
    other.id_ = kNullValue;
    return *this;
  }

  ResourceHandle& operator++()
  {
    ++id_;
    return *this;
  }

  constexpr IdentifierT id() const { return id_; }

  constexpr bool isNull() const { return id_ == kNullValue; }

  constexpr bool isValid() const { return id_ != kNullValue; }

  constexpr operator bool() const { return isValid(); }

  static constexpr T null() { return T{kNullValue}; }

private:
  IdentifierT id_ = kNullValue;
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
  if (handle.isNull())
  {
    return os << "{ id: <NULL> }";
  }
  else
  {
    return os << "{ id: " << handle.id() << " }";
  }
}

}  // namespace sde
