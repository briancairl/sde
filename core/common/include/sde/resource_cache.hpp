/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <unordered_map>

// SDE
#include "sde/crtp.hpp"
#include "sde/expected.hpp"
#include "sde/resource_handle.hpp"

namespace sde
{

template <typename ResourceCacheT> struct ResourceCacheTypes;

template <typename ResourceCacheT> class ResourceCache : public crtp_base<ResourceCache<ResourceCacheT>>
{
public:
  using type_info = ResourceCacheTypes<ResourceCacheT>;
  using error_type = typename type_info::error_type;
  using handle_type = typename type_info::handle_type;
  using value_type = typename type_info::value_type;

  struct element_type
  {
    handle_type handle;
    const value_type* value;

    operator handle_type() const { return handle; }
    operator const value_type&() const { return (*value); }
  };

  using CacheMap = std::unordered_map<handle_type, value_type, ResourceHandleHash>;

  template <typename... CreateArgTs> [[nodiscard]] expected<element_type, error_type> create(CreateArgTs&&... args)
  {
    auto h = handle_lower_bound_;
    ++h;
    return this->add(h, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_type, error_type> add(const handle_type& handle, CreateArgTs&&... args)
  {
    // Create a new element
    auto value_or_error = this->derived().generate(std::forward<CreateArgTs>(args)...);
    if (!value_or_error.has_value())
    {
      return make_unexpected(value_or_error.error());
    }

    // Add it to the cache
    const auto [itr, added] = handle_to_value_cache_.emplace(handle, std::move(value_or_error).value());
    if (added)
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      return element_type{itr->first, std::addressof(itr->second)};
    }

    // Element was a duplicate
    return make_unexpected(error_type::kElementAlreadyExists);
  }

  [[nodiscard]] const value_type* get_if(const handle_type& handle) const
  {
    if (auto itr = handle_to_value_cache_.find(handle); itr != std::end(handle_to_value_cache_))
    {
      return std::addressof(itr->second);
    }
    return nullptr;
  }

  [[nodiscard]] const value_type* operator()(const handle_type& handle) const { return get_if(handle); }

  [[nodiscard]] const bool exists(handle_type handle) const { return handle_to_value_cache_.count(handle) != 0; }

  void remove(handle_type handle) { handle_to_value_cache_.erase(handle); }

  [[nodiscard]] const auto& cache() const { return handle_to_value_cache_; }

  [[nodiscard]] const auto begin() const { return std::begin(handle_to_value_cache_); }

  [[nodiscard]] const auto end() const { return std::end(handle_to_value_cache_); }


  ResourceCache() = default;
  ResourceCache(ResourceCache&&) = default;
  ResourceCache& operator=(ResourceCache&&) = default;

private:
  ResourceCache(const ResourceCache&) = delete;
  ResourceCache& operator=(const ResourceCache&) = delete;
  /// Last used resource handle
  handle_type handle_lower_bound_ = handle_type::null();
  /// Map of {resource_handle, resource_value} objects
  CacheMap handle_to_value_cache_;
};

template <typename ResourceCacheT> struct ElementType
{
  using type = typename ResourceCache<ResourceCacheT>::element_type;
};

template <typename ResourceCacheT> using element_t = typename ElementType<ResourceCacheT>::type;

template <typename ResourceCacheT>
struct is_resource_cache
    : std::integral_constant<bool, std::is_base_of_v<ResourceCache<ResourceCacheT>, ResourceCacheT>>
{};

template <typename ResourceCacheT> constexpr bool is_resource_cache_v = is_resource_cache<ResourceCacheT>::value;

}  // namespace sde
