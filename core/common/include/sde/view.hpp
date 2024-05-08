/**
 * @copyright 2024-present Brian Cairl
 *
 * @file expected.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>

// SDE
#include "sde/crtp.hpp"

namespace sde
{

template <typename ViewT> class BasicView : crtp_base<BasicView<ViewT>>
{
public:
  constexpr bool is_valid() const { return this->derived().data() != nullptr; }
  constexpr operator bool() const { return is_valid(); }

  constexpr auto* begin() { return this->derived().data(); }
  constexpr auto* end() { return this->derived().data() + this->derived().size(); }
  constexpr const auto* begin() const { return this->derived().data(); }
  constexpr const auto* end() const { return this->derived().data().size(); }

private:
  // BasicView() = delete;
};

template <typename T, std::size_t Len = 0UL> class ContinuousView : public BasicView<ContinuousView<T, Len>>
{
public:
  explicit ContinuousView(T* data) : data_{data} {};

  constexpr T* data() { return data_; }
  constexpr const T* data() const { return data_; }
  static constexpr std::size_t size() { return Len; }

private:
  T* data_;
};

template <typename T> class ContinuousView<T, 0> : public BasicView<ContinuousView<T, 0>>
{
public:
  ContinuousView(T* data, std::size_t size) : data_{data}, size_{size} {};

  constexpr T* data() { return data_; }
  constexpr const T* data() const { return data_; }
  constexpr std::size_t size() const { return size_; }

private:
  T* data_;
  std::size_t size_;
};

template <typename T> [[nodiscard]] ContinuousView<T, 0UL> make_view(T* data, std::size_t len)
{
  return ContinuousView<T, 0UL>{data, len};
}

template <std::size_t Len, typename T> [[nodiscard]] ContinuousView<T, Len> make_view(T* data)
{
  return ContinuousView<T, Len>{data};
}

}  // namespace sde
