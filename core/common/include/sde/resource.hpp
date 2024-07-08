/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>

namespace sde
{

template <typename T> struct Field
{
  std::string_view name;
  T& value;
  constexpr Field(std::string_view n, T& v) : name{n}, value{v} {}
};

template <typename L, typename R> constexpr bool operator<(const Field<L>& lhs, const Field<R>& rhs)
{
  return std::addressof(lhs.value) < std::addressof(rhs.value);
}

template <typename L, typename R> constexpr bool operator==(const Field<L>& lhs, const Field<R>& rhs)
{
  return lhs.value == rhs.value;
}

template <typename F> struct is_field : std::false_type
{};

template <typename T> struct is_field<Field<T>> : std::true_type
{};

template <typename... MaybeFieldTs> constexpr bool is_valid_fields_list(const std::tuple<MaybeFieldTs...>& fields)
{
  return (is_field<MaybeFieldTs>() and ...);
}

template <typename... ValueTs> auto to_const(const std::tuple<Field<ValueTs>...>& fields)
{
  return std::apply(
    [](auto&... field) {
      return std::make_tuple(Field<const decltype(field.value)>{field.name, field.value}...);
    },
    fields);
}

template <typename ResourceT> struct Resource
{
  constexpr decltype(auto) fields()
  {
    auto fs = reinterpret_cast<ResourceT*>(this)->fields_list();
    static_assert(is_valid_fields_list(fs));
    return fs;
  }

  constexpr decltype(auto) fields() const
  {
    auto fs = to_const(reinterpret_cast<ResourceT*>(const_cast<Resource*>(this))->fields_list());
    static_assert(is_valid_fields_list(fs));
    return fs;
  }

  constexpr auto values()
  {
    return std::apply(
      [](auto... field) -> std::tuple<decltype(field.value)...> { return {field.value...}; }, this->fields());
  }

  constexpr auto values() const
  {
    return std::apply(
      [](auto... field) -> std::tuple<decltype(field.value)...> { return {field.value...}; }, this->fields());
  }

  constexpr auto names() const
  {
    return std::apply([](auto... field) { return std::make_tuple(field.name...); }, this->fields());
  }
};

}  // namespace sde
