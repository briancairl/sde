/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>

// SDE
#include "sde/crtp.hpp"
#include "sde/hash.hpp"

namespace sde
{

template <typename T> struct BasicField
{
  static_assert(!std::is_reference_v<T> and !std::is_pointer_v<T>);

  const char* name;
  T* value;

  constexpr T& get() { return *value; }
  constexpr const T& get() const { return *value; }

  constexpr T& operator*() { return get(); }
  constexpr const T& operator*() const { return get(); }

  constexpr T* operator->() { return value; }
  constexpr const T* operator->() const { return value; }

  constexpr BasicField(const char* _name, T* _value) : name{_name}, value{_value} {}
};

template <typename L, typename R> constexpr bool operator==(const BasicField<L>& lhs, const BasicField<R>& rhs)
{
  return lhs.get() == rhs.get();
}

template <typename T> struct Field : BasicField<T>
{
  constexpr Field(const char* _name, T& _value) : BasicField<T>{_name, std::addressof(_value)} {}
};

template <typename T> struct _Stub : BasicField<T>
{
  constexpr _Stub(const char* _name, T& _value) : BasicField<T>{_name, std::addressof(_value)} {}
};

template <typename F> struct is_field : std::false_type
{};

template <typename T> struct is_field<Field<T>> : std::true_type
{};

template <typename T> struct is_field<_Stub<T>> : std::true_type
{};

template <typename F> struct is_field_serializable : std::false_type
{};

template <typename T> struct is_field_serializable<Field<T>> : std::true_type
{};

template <typename T> struct is_field_serializable<_Stub<T>> : std::false_type
{};

template <typename T> auto to_const(Field<T> field) { return Field<const T>{field.name, field.get()}; }

template <typename T> auto to_const(_Stub<T> stub) { return _Stub<const T>{stub.name, stub.get()}; }

template <typename... FieldTs> auto to_const(const std::tuple<FieldTs...>& fields)
{
  return std::apply([](auto&... field) { return std::make_tuple(to_const(field)...); }, fields);
}

template <typename ResourceT> struct Resource : crtp_base<Resource<ResourceT>>
{
  constexpr auto fields() { return this->derived().field_list(); }

  constexpr auto fields() const { return to_const(const_cast<Resource*>(this)->derived().field_list()); }

  constexpr auto values()
  {
    return std::apply(
      [](auto... field) -> std::tuple<decltype(field.get())...> { return {field.get()...}; }, this->fields());
  }

  constexpr auto values() const
  {
    return std::apply(
      [](auto... field) -> std::tuple<decltype(field.get())...> { return {field.get()...}; }, this->fields());
  }

  constexpr auto names() const
  {
    return std::apply([](auto... field) { return std::make_tuple(field.name...); }, this->fields());
  }
};

template <typename ResourceT> bool operator==(const Resource<ResourceT>& lhs, const Resource<ResourceT>& rhs)
{
  return lhs.values() == rhs.values();
}

template <typename ResourceT> bool operator!=(const Resource<ResourceT>& lhs, const Resource<ResourceT>& rhs)
{
  return lhs.values() != rhs.values();
}

template <typename ResourceT> bool operator<(const Resource<ResourceT>& lhs, const Resource<ResourceT>& rhs)
{
  return lhs.values() < rhs.values();
}

template <typename ResourceT> bool operator>(const Resource<ResourceT>& lhs, const Resource<ResourceT>& rhs)
{
  return lhs.values() > rhs.values();
}

template <typename ResourceT> bool operator<=(const Resource<ResourceT>& lhs, const Resource<ResourceT>& rhs)
{
  return (lhs < rhs) or (lhs == rhs);
}

template <typename ResourceT> bool operator>=(const Resource<ResourceT>& lhs, const Resource<ResourceT>& rhs)
{
  return (lhs > rhs) or (lhs == rhs);
}

template <typename T> struct is_resource : std::is_base_of<Resource<T>, T>
{};

template <typename R> constexpr bool is_resource_v = is_resource<std::remove_const_t<R>>::value;

struct ResourceHasher
{
  template <typename ResourceT> auto operator()(const Resource<ResourceT>& rsc) const
  {
    return std::apply([](const auto&... fields) { return HashMany(fields...); }, rsc.fields());
  }
};

template <typename T> struct Hasher<Resource<T>> : ResourceHasher
{};

template <typename T> struct Hasher<Field<T>>
{
  Hash operator()(const Field<T>& rsc) const { return Hasher<hashable_t<T>>{}(rsc.get()); }
};

template <typename T> struct Hasher<_Stub<T>>
{
  Hash operator()(const _Stub<T>& rsc) const { return {}; }
};

template <typename... FieldTs> auto FieldList(FieldTs&&... fields)
{
  static_assert((is_field<std::remove_reference_t<FieldTs>>() and ...), "Invalid FieldList");
  return std::make_tuple(std::forward<FieldTs>(fields)...);
}

template <typename ResourceT> auto& _R(Resource<ResourceT>& resource) { return resource; }

template <typename ResourceT> const auto& _R(const Resource<ResourceT>& resource) { return resource; }

namespace detail
{

template <typename T, typename Eval = decltype((std::declval<std::ostream&>() << std::declval<const T&>()))>
constexpr bool call_ostream_overload([[maybe_unused]] const T* _)
{
  return true;
}

constexpr bool call_ostream_overload(...) { return false; }

}  // namespace detail

template <typename T>
using has_ostream_overload =
  std::integral_constant<bool, detail::call_ostream_overload(std::add_pointer_t<T>{nullptr})>;


template <typename T> std::ostream& operator<<(std::ostream& os, const Field<T>& field)
{
  if constexpr (has_ostream_overload<T>())
  {
    return os << field.name << ": " << field.get();
  }
  else
  {
    return os << field.name << ": ...";
  }
}

template <typename T> std::ostream& operator<<(std::ostream& os, const _Stub<T>& field)
{
  if constexpr (has_ostream_overload<T>())
  {
    return os << field.name << ": [?] " << field.get();
  }
  else
  {
    return os << field.name << ": [?] ...";
  }
}

template <typename ResourceT> std::ostream& operator<<(std::ostream& os, const Resource<ResourceT>& rsc)
{
  os << '{';
  std::apply(
    [&os](const auto&... fields) { [[maybe_unused]] auto _ = (((os << fields << ", "), 0) + ...); }, rsc.fields());
  os << '}';
  return os;
}

}  // namespace sde
