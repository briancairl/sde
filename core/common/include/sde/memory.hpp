/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <cstdlib>
#include <memory>
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

template <typename T, typename Alloc = allocator<T>> struct unique_ptr
{
  struct deleter
  {
    void operator()(T* p) { allocator.deallocate(p, sizeof(T)); }

    Alloc allocator;

    explicit deleter(Alloc&& a) : allocator{std::move(a)} {}
  };

  unique_ptr() = default;

  unique_ptr(std::nullptr_t) : p{nullptr} {};

  unique_ptr(std::unique_ptr<T, deleter>&& _p) : p{std::move(_p)} {};

  std::unique_ptr<T, deleter> p;

  operator bool() const { return static_cast<bool>(p); }

  T& operator*() { return *p; }
  const T& operator*() const { return *p; }

  T* operator->() { return p.get(); }
  const T* operator->() const { return p.get(); }
};

template <typename T, typename Alloc, typename OtherT> bool operator!=(const unique_ptr<T, Alloc>& lhs, OtherT&& other)
{
  return lhs.p != std::forward<OtherT>(other);
}

template <typename T, typename Alloc, typename OtherT> bool operator!=(OtherT&& other, const unique_ptr<T, Alloc>& rhs)
{
  return std::forward<OtherT>(other) != rhs.p;
}

template <typename T, typename Alloc, typename OtherT> bool operator==(const unique_ptr<T, Alloc>& lhs, OtherT&& other)
{
  return lhs.p == std::forward<OtherT>(other);
}

template <typename T, typename Alloc, typename OtherT> bool operator==(OtherT&& other, const unique_ptr<T, Alloc>& rhs)
{
  return std::forward<OtherT>(other) == rhs.p;
}


template <typename T, typename Alloc = allocator<T>, typename... Args>
unique_ptr<T, Alloc> allocate_unique(Alloc a, Args&&... args)
{
  using uptr = unique_ptr<T, Alloc>;
  using deleter = typename uptr::deleter;

  Alloc allocator;
  T* p_raw = allocator.allocate(sizeof(T));
  new (p_raw) T(std::forward<Args>()...);
  return uptr{std::unique_ptr<T, deleter>{p_raw, deleter{std::move(allocator)}}};
}

template <typename T, typename Alloc = allocator<T>, typename... Args> unique_ptr<T, Alloc> make_unique(Args&&... args)
{
  return allocate_unique<T, Alloc, Args...>(Alloc{}, std::forward<Args>(args)...);
}

}  // namespace sde
