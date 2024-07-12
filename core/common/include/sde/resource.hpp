/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>

// SDE
#include "sde/hash.hpp"

namespace sde
{

template <typename T> struct BasicField
{
  std::string_view name;
  T& value;
  constexpr BasicField(std::string_view n, T& v) : name{n}, value{v} {}
};

template <typename L, typename R> constexpr bool operator==(const BasicField<L>& lhs, const BasicField<R>& rhs)
{
  return lhs.value == rhs.value;
}

template <typename T> struct Field : BasicField<T>
{
  constexpr Field(std::string_view n, T& v) : BasicField<T>{n, v} {}
};

template <typename T> struct FieldNotSerialized : BasicField<T>
{
  constexpr FieldNotSerialized(std::string_view n, T& v) : BasicField<T>{n, v} {}
};

struct NotSerializedTag
{};

static constexpr NotSerializedTag kNotSerialized{};

template <typename T> constexpr FieldNotSerialized<T> operator|(Field<T> field, NotSerializedTag _)
{
  return FieldNotSerialized<T>{field.name, field.value};
}

template <typename F> struct is_field : std::false_type
{};

template <typename T> struct is_field<Field<T>> : std::true_type
{};

template <typename T> struct is_field<FieldNotSerialized<T>> : std::true_type
{};

template <typename F> struct is_field_serializable : std::false_type
{};

template <typename T> struct is_field_serializable<Field<T>> : std::true_type
{};

template <typename T> struct is_field_serializable<FieldNotSerialized<T>> : std::false_type
{};

template <typename F> struct to_const_field;

template <typename T> struct to_const_field<Field<T>>
{
  using type = Field<const std::remove_const_t<T>>;
};

template <typename T> struct to_const_field<FieldNotSerialized<T>>
{
  using type = FieldNotSerialized<const std::remove_const_t<T>>;
};

template <typename T> using to_const_field_t = typename to_const_field<T>::type;

template <typename... MaybeFieldTs> constexpr bool is_valid_fields_list(const std::tuple<MaybeFieldTs...>& fields)
{
  return (is_field<MaybeFieldTs>() and ...);
}

template <typename... FieldTs> auto to_const(const std::tuple<FieldTs...>& fields)
{
  return std::apply(
    [](auto&... field) {
      return std::make_tuple(to_const_field_t<FieldTs>{field.name, field.value}...);
    },
    fields);
}

template <typename ResourceT> struct Resource
{
  auto& fundemental() { return *this; }

  const auto& fundemental() const { return *this; }

  constexpr decltype(auto) fields()
  {
    auto fields = reinterpret_cast<ResourceT*>(this)->fields_list();
    static_assert(is_valid_fields_list(fields));
    return fields;
  }

  constexpr decltype(auto) fields() const
  {
    auto fields = to_const(reinterpret_cast<ResourceT*>(const_cast<Resource*>(this))->fields_list());
    static_assert(is_valid_fields_list(fields));
    return fields;
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

struct ResourceHasher
{
  template <typename ResourceT> std::size_t operator()(const Resource<ResourceT>& rsc) const
  {
    return std::apply([](const auto&... fields) { return HashMultiple(fields...); }, rsc.fields());
  }
};

template <typename ResourceT> struct Hasher<Resource<ResourceT>> : ResourceHasher
{};

template <typename T> struct Hasher<Field<T>>
{
  std::size_t operator()(const Field<T>& rsc) const { return Hasher<std::remove_const_t<T>>{}(rsc.value); }
};

template <typename T> struct Hasher<FieldNotSerialized<T>>
{
  std::size_t operator()(const FieldNotSerialized<T>& rsc) const { return 0xDEADBEEFBEEFDEAD; }
};

template <typename T> std::ostream& operator<<(std::ostream& os, const Field<T>& field)
{
  return os << field.name << ": " << field.value;
}

template <typename T> std::ostream& operator<<(std::ostream& os, const FieldNotSerialized<T>& field)
{
  return os << field.name << ": [?] " << field.value;
}


template <typename ResourceT> std::ostream& operator<<(std::ostream& os, const Resource<ResourceT>& rsc)
{
  os << '{';
  std::apply(
    [&os](const auto&... fields) { [[maybe_unused]] auto _ = (((os << fields << ' '), 0) + ...); }, rsc.fields());
  os << '}';
  return os;
}

}  // namespace sde
