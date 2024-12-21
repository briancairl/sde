/**
 * @copyright 2024-present Brian Cairl
 *
 * @file expected.hpp
 */
#pragma once

// C++ Standard Library
#include <utility>

namespace sde
{

template <typename T> struct DefaultExchanger
{
  void operator()(T& lhs, T& rhs) const { std::swap(lhs, rhs); }
};

template <typename T, typename DeleterT, T kNullValue = static_cast<T>(0)> class UniqueResource
{
public:
  explicit UniqueResource(const T& v, DeleterT deleter = DeleterT{}) : value_{v}, deleter_{std::move(deleter)} {}

  UniqueResource(UniqueResource&& other) { this->swap(other); }

  ~UniqueResource()
  {
    if (value_ != kNullValue)
    {
      deleter_(value_);
    }
  }

  UniqueResource& operator=(UniqueResource&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(UniqueResource& other)
  {
    std::swap(value_, other.value_);
    std::swap(deleter_, other.deleter_);
  }

  [[nodiscard]] constexpr const T operator->() const
  {
    static_assert(std::is_pointer_v<T>);
    return value_;
  }

  [[nodiscard]] constexpr const T& operator*() const
  {
    static_assert(std::is_pointer_v<T>);
    return *value_;
  }

  [[nodiscard]] constexpr const T& value() const { return value_; }

  [[nodiscard]] constexpr operator const T&() const { return value_; }

  [[nodiscard]] constexpr bool isValid() const { return value_ != kNullValue; }

  [[nodiscard]] constexpr bool isNull() const { return value_ == kNullValue; }

private:
  UniqueResource(const UniqueResource&) = delete;
  UniqueResource& operator=(const UniqueResource&) = delete;

  T value_ = kNullValue;
  DeleterT deleter_;
};

template <typename T, typename DeleterT, T kNullValue>
constexpr bool
operator==(const UniqueResource<T, DeleterT, kNullValue>& lhs, const UniqueResource<T, DeleterT, kNullValue>& rhs)
{
  return lhs.value() == rhs.value();
}

template <typename T, typename DeleterT, T kNullValue>
constexpr bool operator==([[maybe_unused]] std::nullptr_t _, const UniqueResource<T, DeleterT, kNullValue>& rhs)
{
  return rhs.isNull();
}

template <typename T, typename DeleterT, T kNullValue>
constexpr bool operator==(const UniqueResource<T, DeleterT, kNullValue>& lhs, [[maybe_unused]] std::nullptr_t _)
{
  return lhs.isNull();
}

template <typename T, typename DeleterT, T kNullValue>
constexpr bool
operator!=(const UniqueResource<T, DeleterT, kNullValue>& lhs, const UniqueResource<T, DeleterT, kNullValue>& rhs)
{
  return lhs.value() != rhs.value();
}

template <typename T, typename DeleterT, T kNullValue>
constexpr bool operator!=([[maybe_unused]] std::nullptr_t _, const UniqueResource<T, DeleterT, kNullValue>& rhs)
{
  return rhs.isValid();
}

template <typename T, typename DeleterT, T kNullValue>
constexpr bool operator!=(const UniqueResource<T, DeleterT, kNullValue>& lhs, [[maybe_unused]] std::nullptr_t _)
{
  return lhs.isValid();
}

}  // namespace sde
