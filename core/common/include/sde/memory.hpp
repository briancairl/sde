/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <cstdlib>
#include <type_traits>

namespace sde
{

using MallocImpl = void*(std::size_t);
using FreeImpl = void(void*);

template <typename T> class allocator
{
public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using propagate_on_container_move_assignment = std::true_type;

  allocator(MallocImpl m = std::malloc, FreeImpl f = std::free) : malloc_impl_{m}, free_impl_{f} {}

  allocator(allocator&&) = default;
  allocator(const allocator&) = default;
  allocator& operator=(allocator&&) = default;
  allocator& operator=(const allocator&) = default;

  template <typename U>
  allocator(const allocator<U>& other) : malloc_impl_{other.malloc_impl_}, free_impl_{other.free_impl_}
  {}

  T* allocate(std::size_t n) { return reinterpret_cast<T*>(malloc_impl_(n * sizeof(T))); }

  void deallocate(T* p, [[maybe_unused]] std::size_t n) { free_impl_(reinterpret_cast<void*>(p)); }

  constexpr MallocImpl* malloc_impl() const { return malloc_impl_; }
  constexpr FreeImpl* free_impl() const { return free_impl_; }

private:
  template <typename U> friend class allocator;
  MallocImpl* malloc_impl_;
  FreeImpl* free_impl_;
};

template <typename T1, typename T2> bool operator==(const allocator<T1>& lhs, const allocator<T2>& rhs)
{
  return (lhs.malloc_impl() == rhs.malloc_impl()) && (lhs.free_impl() == rhs.free_impl());
}

template <typename T1, typename T2> bool operator!=(const allocator<T1>& lhs, const allocator<T2>& rhs)
{
  return (lhs.malloc_impl() != rhs.malloc_impl()) || (lhs.free_impl() != rhs.free_impl());
}

}  // namespace sde
