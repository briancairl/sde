#pragma once

// Dont
#include <dont/conditions.hpp>

// SDE
#include "sde/resource_handle.hpp"

namespace sde
{

template <typename... DependTs> struct ResourceDependencies
{
public:
  static constexpr bool kDoNotHash{true};

  ResourceDependencies() = default;

  explicit ResourceDependencies(std::add_lvalue_reference_t<DependTs>&... deps) : tup_{std::addressof(deps)...} {}

  template <typename... ParentDependTs>
  ResourceDependencies(ResourceDependencies<ParentDependTs...> parent) :
      ResourceDependencies{parent.template get<DependTs>()...}
  {
    static_assert(
      (dont::IsMember<DependTs, ResourceDependencies<ParentDependTs...>>::value && ...),
      "DependTs must be a subset of ParentDependTs");
  }

  template <typename T> auto& get() { return *std::template get<std::add_pointer_t<T>>(tup_); }

  template <typename T> const auto& get() const { return *std::template get<std::add_pointer_t<T>>(tup_); }

  template <typename HandleT> decltype(auto) borrow(const HandleT& handle)
  {
    static_assert(is_resource_handle_v<HandleT>);
    return this->template get<resource_handle_to_cache_t<HandleT>>().borrow(handle);
  }

  template <typename HandleT> decltype(auto) restore(const HandleT& handle)
  {
    static_assert(is_resource_handle_v<HandleT>);
    return this->template get<resource_handle_to_cache_t<HandleT>>().restore(handle);
  }

  template <typename HandleT> decltype(auto) operator()(const HandleT& handle)
  {
    static_assert(is_resource_handle_v<HandleT>);
    return this->template get<resource_handle_to_cache_t<HandleT>>()(handle);
  }

  template <typename HandleT> decltype(auto) operator()(const HandleT& handle) const
  {
    static_assert(is_resource_handle_v<HandleT>);
    return this->template get<resource_handle_to_cache_t<HandleT>>()(handle);
  }

  static constexpr auto size() { return sizeof...(DependTs); }

private:
  std::tuple<std::add_pointer_t<DependTs>...> tup_;
};

template <typename... DependTs> ResourceDependencies(DependTs&...) -> ResourceDependencies<DependTs...>;

template <> struct ResourceDependencies<>
{
  static constexpr bool kDoNotHash{true};
  static constexpr auto size() { return 0; }

  ResourceDependencies() = default;

  template <typename... ParentDependTs>
  ResourceDependencies([[maybe_unused]] ResourceDependencies<ParentDependTs...> parent)
  {}
};

using no_dependencies = ResourceDependencies<>;

constexpr auto NoDependencies = no_dependencies{};

}  // namespace sde