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
#include "sde/hash.hpp"

namespace sde
{

template <typename T> struct ResourceHandleBase
{};

template <typename T, typename IdentifierT = std::size_t, IdentifierT kNullValue = 0>
struct ResourceHandle : ResourceHandleBase<T>
{
public:
  using id_type = IdentifierT;
  using self_type = ResourceHandle<T, id_type, kNullValue>;

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

  self_type& fundemental() { return *this; }

  const self_type& fundemental() const { return *this; }

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

template <typename T> struct is_resource_handle : std::is_base_of<ResourceHandleBase<T>, T>
{};

template <typename H> constexpr bool is_resource_handle_v = is_resource_handle<std::remove_const_t<H>>::value;


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

template <typename T, typename IdentifierT, IdentifierT kNullValue>
auto& _R(ResourceHandle<T, IdentifierT, kNullValue>& handle)
{
  return handle;
}

template <typename T, typename IdentifierT, IdentifierT kNullValue>
const auto& _R(const ResourceHandle<T, IdentifierT, kNullValue>& handle)
{
  return handle;
}

}  // namespace sde
