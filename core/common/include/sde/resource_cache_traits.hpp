#pragma once

// Dont
#include <dont/conditions.hpp>

namespace sde
{

template <typename... DependTs> struct ResourceDependencies
{
public:
  static constexpr bool kDoNotHash{true};

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

  static constexpr auto size() { return sizeof...(DependTs); }

private:
  std::tuple<std::add_pointer_t<DependTs>...> tup_;
};

template <typename... DependTs> ResourceDependencies(DependTs&...) -> ResourceDependencies<DependTs...>;

template <> struct ResourceDependencies<>
{
  static constexpr bool kDoNotHash{true};
  static constexpr auto size() { return 0; }
};

using no_dependencies = ResourceDependencies<>;

constexpr auto NoDependencies = no_dependencies{};

template <typename ResourceCacheT> struct ResourceCacheTraits
{
  using error_type = void;
  using handle_type = void;
  using value_type = void;
  using dependencies = no_dependencies;
};

}  // namespace sde