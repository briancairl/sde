/**
 * @copyright 2024-present Brian Cairl
 *
 * @file view.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <cstdint>

// SDE
#include "sde/crtp.hpp"

namespace sde
{

template <typename ViewT> class BasicView : public crtp_base<BasicView<ViewT>>
{
  friend class fundemental_type;

public:
  constexpr bool isValid() const { return this->derived().data() != nullptr; }
  constexpr operator bool() const { return isValid(); }

  constexpr std::size_t size() const { return this->derived().length(); }
  constexpr auto* begin() { return this->derived().data(); }
  constexpr auto* end() { return this->derived().data() + this->size(); }
  constexpr const auto* begin() const { return this->derived().data(); }
  constexpr const auto* end() const { return this->derived().data() + this->size(); }

  constexpr bool empty() const { return begin() == end(); }

private:
  // BasicView() = delete;
};

template <typename ViewT> bool operator==(const BasicView<ViewT>& lhs, const BasicView<ViewT>& rhs)
{
  return (lhs.size() == rhs.size()) && std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

template <typename T, std::size_t Len = 0UL> class View : public BasicView<View<T, Len>>
{
  friend class BasicView<View<T, Len>>;

public:
  explicit View(T* data) : data_{data} {};

  constexpr T* data() { return data_; }
  constexpr const T* data() const { return data_; }
  static constexpr std::size_t length() { return Len; }

private:
  T* data_;
};

template <typename T> class View<T, 0> : public BasicView<View<T, 0>>
{
  friend class BasicView<View<T, 0>>;

public:
  explicit View([[maybe_unused]] std::nullptr_t _) : data_{nullptr}, size_{0} {};

  explicit View(T* data = nullptr, std::size_t size = 0UL) : data_{data}, size_{size} {};

  constexpr T* data() { return data_; }
  constexpr const T* data() const { return data_; }
  constexpr std::size_t length() const { return size_; }

private:
  T* data_;
  std::size_t size_;
};

template <typename T> [[nodiscard]] View<T> make_view(T* data, std::size_t len) { return View<T>{data, len}; }
template <std::size_t Len, typename T> [[nodiscard]] View<T, Len> make_view(T* data) { return View<T, Len>{data}; }

template <std::size_t Len, typename T> [[nodiscard]] View<const T, Len> make_const_view(const T* data)
{
  return View<const T, Len>{data};
}
template <typename T> [[nodiscard]] View<const T> make_const_view(const T* data, std::size_t len)
{
  return View<const T>{data, len};
}

template <template <class, class...> class Container, typename T, typename... OtherTs>
[[nodiscard]] auto make_view(Container<T, OtherTs...>& container)
{
  return View<T>{container.data(), container.size()};
}

template <template <class, class...> class Container, typename T, typename... OtherTs>
[[nodiscard]] auto make_const_view(const Container<T, OtherTs...>& container)
{
  return View<std::add_const_t<T>>{container.data(), container.size()};
}

}  // namespace sde
