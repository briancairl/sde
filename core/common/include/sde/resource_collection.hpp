/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_collection.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <tuple>
#include <type_traits>

// Dont
#include <dont/map.hpp>
#include <dont/stl/tuple/for_each.hpp>

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
  static constexpr const char* data() { return str.data; }
  static constexpr size_t size() { return str.size(); }
  static constexpr std::string_view view() { return std::string_view{data(), size()}; }
};

template <resource_label_data str> std::ostream& operator<<(std::ostream& os, const resource_label<str>& label)
{
  return os << "Resource[" << label.view() << ']';
}

template <resource_label_data str> constexpr auto operator"" _rl() { return resource_label<str>{}; }

template <resource_label kName, typename ResourceCacheT, bool _kShouldSerialize = true> struct ResourceCollectionEntry
{
  using type = ResourceCacheT;

  static constexpr bool kShouldSerialize{_kShouldSerialize};

  static constexpr const char* name() { return kName.data(); }

  auto as_field()
  {
    if constexpr (kShouldSerialize)
    {
      return Field{name(), cache};
    }
    else
    {
      return _Stub{name(), cache};
    }
  }

  ResourceCacheT cache;

  ResourceCollectionEntry() = default;

  ResourceCollectionEntry(ResourceCollectionEntry&& other) = default;
  ResourceCollectionEntry& operator=(ResourceCollectionEntry&& other) = default;

  ResourceCollectionEntry(const ResourceCollectionEntry& other) = delete;
  ResourceCollectionEntry& operator=(const ResourceCollectionEntry& other) = delete;
};

template <typename EntryT> struct IsResourceCollectionEntry : std::false_type
{};

template <resource_label kName, typename ResourceCacheT, bool kShouldSerialize>
struct IsResourceCollectionEntry<ResourceCollectionEntry<kName, ResourceCacheT, kShouldSerialize>> : std::true_type
{};


template <typename EntryT> constexpr bool is_resource_collection_entry_v = IsResourceCollectionEntry<EntryT>::value;


template <typename... ResourceCollectionEntryTs>
class ResourceCollection : public Resource<ResourceCollection<ResourceCollectionEntryTs...>>
{
  friend class Resource<ResourceCollection<ResourceCollectionEntryTs...>>;

  static_assert(
    (is_resource_collection_entry_v<ResourceCollectionEntryTs> and ...),
    "ResourceCollectionEntryTs must be a ResourceCollectionEntry<LABLE, CACHE>");

  static_assert(
    (is_resource_cache_like_v<typename ResourceCollectionEntryTs::type> and ...),
    "ResourceCacheTs must be a ResourceCache<T> (or have compatible API)");

public:
  using handle_to_cache_map = dont::Map<dont::KeyValue<
    typename ResourceCacheTraits<typename ResourceCollectionEntryTs::type>::handle_type,
    typename ResourceCollectionEntryTs::type>...>;

  using cache_to_entry_map =
    dont::Map<dont::KeyValue<typename ResourceCollectionEntryTs::type, ResourceCollectionEntryTs>...>;

  template <typename CacheT> constexpr CacheT& get()
  {
    using EntryType = dont::map_lookup_t<cache_to_entry_map, CacheT>;
    return std::get<EntryType>(caches_).cache;
  }

  template <typename CacheT> constexpr const CacheT& get() const
  {
    using EntryType = dont::map_lookup_t<cache_to_entry_map, CacheT>;
    return std::get<EntryType>(caches_).cache;
  }

  constexpr auto all() { return ResourceDependencies{std::get<ResourceCollectionEntryTs>(caches_).cache...}; }

  constexpr operator ResourceDependencies<typename ResourceCollectionEntryTs::type...>()
  {
    return ResourceDependencies{std::get<typename ResourceCollectionEntryTs::type>(caches_)...};
  }

  template <typename CacheT, typename... CreateArgTs> [[nodiscard]] auto create(CreateArgTs&&... args)
  {
    auto& cache = this->template get<CacheT>();
    return cache.create(this->all(), std::forward<CreateArgTs>(args)...);
  }

  template <typename CacheT, typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto find_and_replace_or_create(HandleT&& handle, CreateArgTs&&... args)
  {
    auto& cache = this->template get<CacheT>();
    return cache.find_and_replace_or_create(
      std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto find_or_replace(HandleT&& handle, CreateArgTs&&... args)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    auto& cache = this->template get<CacheType>();
    return cache.find_or_replace(std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
  }

  template <typename CacheT, typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto find_or_create(HandleT&& handle, CreateArgTs&&... args)
  {
    auto& cache = this->template get<CacheT>();
    return cache.find_or_create(std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto emplace_with_hint(HandleT&& handle, CreateArgTs&&... args)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    auto& cache = this->template get<CacheType>();
    return cache.emplace_with_hint(std::forward<HandleT>(handle), this->all(), std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] auto assign(HandleT& handle, CreateArgTs&&... asset_create_args)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;

    expected<ResourceStatus, typename ResourceCacheTraits<CacheType>::error_type> status_or_error;

    auto& cache = this->template get<CacheType>();
    auto handle_and_value_or_error =
      cache.find_or_create(handle, this->all(), std::forward<CreateArgTs>(asset_create_args)...);
    if (handle_and_value_or_error.has_value())
    {
      handle = handle_and_value_or_error->handle;
      status_or_error = handle_and_value_or_error->status;
    }
    else
    {
      status_or_error = make_unexpected(handle_and_value_or_error.error());
    }
    return status_or_error;
  }

  template <typename HandleT> [[nodiscard]] auto get_if(HandleT&& handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    return this->template get<CacheType>().get_if(std::forward<HandleT>(handle));
  }

  template <typename HandleT> [[nodiscard]] auto operator()(HandleT&& handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    return this->template get<CacheType>().find(std::forward<HandleT>(handle));
  }

  template <typename HandleT> [[nodiscard]] auto find(HandleT&& handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    return this->template get<CacheType>().find(std::forward<HandleT>(handle));
  }

  template <typename HandleT> [[nodiscard]] const bool exists(HandleT&& handle) const
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    return this->template get<CacheType>().exists(std::forward<HandleT>(handle));
  }

  template <typename HandleT> void remove(HandleT&& handle)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    this->template get<CacheType>().remove(std::forward<HandleT>(handle), this->all());
  }

  template <typename HandleT, typename UpdateFn> void update_if_exists(HandleT&& handle, UpdateFn update)
  {
    using CacheType = dont::map_lookup_t<handle_to_cache_map, std::remove_const_t<std::remove_reference_t<HandleT>>>;
    this->template get<CacheType>().update_if_exists(std::forward<HandleT>(handle), std::forward<UpdateFn>(update));
  }

  expected<void, std::string_view> refresh()
  {
    expected<void, std::string_view> ok_or_error = {};
    dont::tuple::for_each(
      [this, &ok_or_error](auto& entry) {
        using CacheType = std::remove_reference_t<decltype(entry.cache)>;
        using EntryType = dont::map_lookup_t<cache_to_entry_map, CacheType>;
        if constexpr (EntryType::kShouldSerialize)
        {
          if (auto ok = entry.cache.refresh(this->all()); !ok.has_value())
          {
            ok_or_error = make_unexpected(std::string_view{EntryType::name()});
            return false;
          }
        }
        return true;
      },
      caches_);
    return ok_or_error;
  }

  void swap(ResourceCollection& other) { std::swap(this->caches_, other.caches_); }

  void clear()
  {
    dont::tuple::for_each(
      [deps = this->all()](auto& entry) { entry.cache.clear(deps); }, dont::tuple::reversed(caches_));
  }

  ~ResourceCollection() { this->clear(); }

  ResourceCollection() = default;

  ResourceCollection(ResourceCollection&&) = default;
  ResourceCollection& operator=(ResourceCollection&&) = default;

  ResourceCollection(const ResourceCollection&) = delete;
  ResourceCollection& operator=(const ResourceCollection&) = delete;

private:
  std::tuple<ResourceCollectionEntryTs...> caches_;

  auto field_list() { return FieldList(std::get<ResourceCollectionEntryTs>(caches_).as_field()...); }
};

}  // namespace sde
