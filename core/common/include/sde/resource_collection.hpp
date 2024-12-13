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

template <std::size_t Len> struct resource_label_data
{
  char data[Len];
  constexpr std::size_t size() const { return Len - 1; }
  constexpr resource_label_data(const char (&init)[Len]) { std::copy_n(init, Len, data); }
};

template <auto str> struct resource_label
{
  using type_string_tag = void;
  static constexpr const char* data() { return str.data; }
  static constexpr size_t size() { return str.size(); }
  static constexpr std::string_view view() { return std::string_view{data(), size()}; }
};

template <resource_label_data str> constexpr auto operator"" _label() { return resource_label<str>{}; }

template <resource_label kName, typename ResourceCacheT> struct ResourceCollectionEntry
{
  using type = ResourceCacheT;
  static constexpr const char* name() { return kName.data(); }
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
