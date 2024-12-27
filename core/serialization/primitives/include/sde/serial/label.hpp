/**
 * @copyright 2022-present Brian Cairl
 *
 * @file label.hpp
 */
#pragma once

// C++ Standard Library
#include <string_view>
#include <type_traits>

namespace sde::serial
{

template <typename T> struct label
{
  std::string_view value;
};

template <typename T> struct is_label : std::false_type
{};

template <typename T> struct is_label<label<T>> : std::true_type
{};

template <typename T>
static constexpr bool is_label_v = is_label<std::remove_const_t<std::remove_reference_t<T>>>::value;

}  // namespace sde::serial
