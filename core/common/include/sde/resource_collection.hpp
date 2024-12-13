/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_collection.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>
#include <type_traits>

// Dont
#include <dont/map.hpp>

// SDE
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_handle.hpp"

namespace sde
{

/**
 * @brief Holds characters in a variadic pack
 *
 * Acts as a string literal creator/wrapper with safeguards against invalid
 * c-string points
 *
 * @tparam Elements  characters of string
 */
template <char... Elements> struct ResourceLabel
{
public:
  /**
   * @brief Returns C-string comprised of <code>{Elements...}</code>
   */
  static constexpr const char* name() { return str_storage; }

private:
  /// String literal constant, formed from Elements
  static constexpr char str_storage[] = {Elements..., '\0'};
};

template <typename T, T... Elements> constexpr auto operator""_label() { return ResourceLabel<Elements...>{}; }

template <typename LabelT, typename ResourceCacheT> struct ResourceCollectionEntry;

template <char... C, typename ResourceCacheT>
struct ResourceCollectionEntry<ResourceLabel<C...>, ResourceCacheT> : ResourceLabel<C...>
{
  using type = ResourceCacheT;
};

template <typename... ResourceCollectionEntryTs>
class ResourceCollection : public Resource<ResourceCollection<ResourceCollectionEntryTs...>>
{
  friend class fundemental_type;

public:
  static_assert(
    (is_resource_cache_v<typename ResourceCollectionEntryTs::type> && ...),
    "ResourceCacheTs must be a ResourceCache<T>");

  using handle_to_cache_map = dont::Map<dont::KeyValue<
    typename ResourceCacheTraits<typename ResourceCollectionEntryTs::type>::handle_type,
    typename ResourceCollectionEntryTs::type>...>;

  template <typename CacheT> constexpr CacheT& get() { return std::get<CacheT>(caches_); }

  template <typename CacheT> constexpr const CacheT& get() const { return std::get<CacheT>(caches_); }

  constexpr auto all() { return ResourceDependencies{std::get<typename ResourceCollectionEntryTs::type>(caches_)...}; }

  constexpr operator ResourceDependencies<typename ResourceCollectionEntryTs::type...>()
  {
    return ResourceDependencies{std::get<typename ResourceCollectionEntryTs::type>(caches_)...};
  }


  template <typename CacheT, typename... CreateArgTs> [[nodiscard]] auto create(CreateArgTs&&... args)
  {
    if constexpr (resource_cache_has_dependencies_v<CacheT>)
    {
      return this->template get<CacheT>().create(this->all(), std::forward<CreateArgTs>(args)...);
    }
    else
    {
      return this->template get<CacheT>().create(std::forward<CreateArgTs>(args)...);
    }
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto find_or_replace(HandleT handle, CreateArgTs&&... args)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    if constexpr (resource_cache_has_dependencies_v<CacheType>)
    {
      return this->template get<CacheType>().find_or_replace(
        std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
    }
    else
    {
      return this->template get<CacheType>().find_or_replace(
        std::forward<HandleT>(handle), std::forward<CreateArgTs>(args)...);
    }
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto find_or_create(HandleT handle, CreateArgTs&&... args)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    if constexpr (resource_cache_has_dependencies_v<CacheType>)
    {
      return this->template get<CacheType>().find_or_create(
        std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
    }
    else
    {
      return this->template get<CacheType>().find_or_create(
        std::forward<HandleT>(handle), std::forward<CreateArgTs>(args)...);
    }
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto emplace_with_hint(HandleT handle, CreateArgTs&&... args)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    if constexpr (resource_cache_has_dependencies_v<CacheType>)
    {
      return this->template get<CacheType>().emplace_with_hint(
        std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
    }
    else
    {
      return this->template get<CacheType>().emplace_with_hint(
        std::forward<HandleT>(handle), std::forward<CreateArgTs>(args)...);
    }
  }

  template <typename HandleT> [[nodiscard]] auto get_if(HandleT handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    return this->template get<CacheType>().get_if(std::forward<HandleT>(handle));
  }

  template <typename HandleT> [[nodiscard]] auto operator()(HandleT handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    return this->template get<CacheType>().find(std::forward<HandleT>(handle));
  }

  template <typename HandleT> [[nodiscard]] auto find(HandleT handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    return this->template get<CacheType>().find(std::forward<HandleT>(handle));
  }

  template <typename HandleT> [[nodiscard]] const bool exists(HandleT handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    return this->template get<CacheType>().exists(std::forward<HandleT>(handle));
  }

  template <typename HandleT> void remove(HandleT handle)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    this->template get<CacheType>().remove(std::forward<HandleT>(handle));
  }

  template <typename HandleT, typename UpdateFn> void update_if_exists(HandleT handle, UpdateFn update)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, HandleT>;
    this->template get<CacheType>().update_if_exists(std::forward<HandleT>(handle), std::forward<UpdateFn>(update));
  }

  void swap(ResourceCollection& other) { std::swap(this->caches_, other.caches_); }

  ResourceCollection() = default;

  ResourceCollection(ResourceCollection&&) = default;
  ResourceCollection& operator=(ResourceCollection&&) = default;

  ResourceCollection(const ResourceCollection&) = delete;
  ResourceCollection& operator=(const ResourceCollection&) = delete;

private:
  std::tuple<typename ResourceCollectionEntryTs::type...> caches_;

  auto field_list()
  {
    return FieldList(
      Field{ResourceCollectionEntryTs::name(), this->template get<typename ResourceCollectionEntryTs::type>()}...);
  }
};

}  // namespace sde
