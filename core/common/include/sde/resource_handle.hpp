/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_handle.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <type_traits>

// SDE
#include "sde/crtp.hpp"
#include "sde/hash.hpp"

namespace sde
{

template <typename T> struct ResourceHandle : crtp_base<ResourceHandle<T>>
{
  friend class fundemental_type;

public:
  using id_type = std::size_t;
  static const id_type kNullValue{0};

  explicit ResourceHandle(id_type id) : id_{id} {}

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

  ResourceHandle& operator=(const id_type& id)
  {
    id_ = id;
    return *this;
  }

  ResourceHandle& operator++()
  {
    id_ = T::next_unique(id_);
    return *this;
  }

  constexpr id_type id() const { return id_; }

  constexpr bool isNull() const { return id_ == kNullValue; }

  constexpr bool isValid() const { return id_ != kNullValue; }

  constexpr operator bool() const { return isValid(); }

  static constexpr T null() { return T{kNullValue}; }

private:
  static constexpr id_type next_unique(id_type prev) { return prev + 1; }
  id_type id_ = kNullValue;
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
  template <typename T> constexpr Hash operator()(const ResourceHandle<T>& handle) const
  {
    return {static_cast<std::size_t>(handle.id())};
  }
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
