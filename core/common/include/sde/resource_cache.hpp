/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <type_traits>

// Dont
#include <dont/merge.hpp>

// SDE
#include "sde/crtp.hpp"
#include "sde/expected.hpp"
#include "sde/hash.hpp"
#include "sde/memory.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache_traits.hpp"
#include "sde/resource_handle.hpp"
#include "sde/unordered_map.hpp"

namespace sde
{
enum class ResourceStatus
{
  kInvalid,
  kCreated,
  kReplaced,
  kExisted
};

template <typename ResourceDependenciesT> struct ExpandResourceDependencies;

template <typename ResourceDependenciesT>
using expand_resource_dependencies_t = typename ExpandResourceDependencies<ResourceDependenciesT>::type;

template <> struct ExpandResourceDependencies<ResourceDependencies<>>
{
  using type = ResourceDependencies<>;
};

template <typename... ResourceCacheTs> struct ExpandResourceDependencies<ResourceDependencies<ResourceCacheTs...>>
{
  using type = dont::set_union_t<
    // Primary dependencies
    ResourceDependencies<ResourceCacheTs...>,
    // Transitive dependencies
    typename ResourceCacheTraits<ResourceCacheTs>::dependencies...,
    // Recursive expansion
    expand_resource_dependencies_t<typename ResourceCacheTraits<ResourceCacheTs>::dependencies>...>;
};

template <typename ResourceCacheT> class ResourceCache : public crtp_base<ResourceCache<ResourceCacheT>>
{
public:
  using dependencies = expand_resource_dependencies_t<typename ResourceCacheTraits<ResourceCacheT>::dependencies>;
  using type_info = ResourceCacheTraits<ResourceCacheT>;
  using error_type = typename type_info::error_type;
  using handle_type = typename type_info::handle_type;
  using value_type = typename type_info::value_type;
  using version_type = Hash;

  struct element_ref
  {
    ResourceStatus status;
    handle_type handle;
    const value_type* value;

    operator bool() const { return value != nullptr; }
    const value_type& get() const { return *value; }
    const value_type& operator*() const { return get(); }
    const value_type* operator->() const { return value; }
  };

  class element_storage : public Resource<element_storage>
  {
  public:
    version_type version;
    value_type value;

    const value_type& get() const { return value; }
    const value_type& operator*() const { return get(); }
    const value_type* operator->() const { return std::addressof(value); }

    element_storage(element_storage&& other) = default;
    element_storage& operator=(element_storage&& other) = default;

    template <typename... ValueArgTs>
    explicit element_storage(version_type _version, ValueArgTs&&... args) :
        version{_version}, value{std::forward<ValueArgTs>(args)...}
    {}

    auto field_list() { return FieldList(Field{"version", version}, Field{"value", value}); }

  private:
    element_storage(const element_storage& other) = delete;
    element_storage& operator=(const element_storage& other) = delete;
  };

  struct handle_type_hash
  {
    std::size_t operator()(const handle_type& h) const { return std::hash<typename handle_type::id_type>{}(h.id()); }
  };

  // clang-format off
  using CacheMap = sde::unordered_map<
    handle_type,
    element_storage,
    handle_type_hash,
    std::equal_to<handle_type>
  >;
  // clang-format on

  template <typename... CreateArgTs> [[nodiscard]] expected<element_ref, error_type> create(CreateArgTs&&... args)
  {
    const auto handle = this->derived().next_unique_id(handle_to_value_cache_, handle_lower_bound_);
    auto element_or_error = this->create_at_handle(handle, std::forward<CreateArgTs>(args)...);
    if (element_or_error.has_value())
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      return std::move(element_or_error).value();
    }
    return make_unexpected(element_or_error.error());
  }

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> find_or_replace(handle_type handle, CreateArgTs&&... args)
  {
    const auto current_itr = handle_to_value_cache_.find(handle);

    if (current_itr == handle_to_value_cache_.end())
    {
      return create_at_handle(handle, std::forward<CreateArgTs>(args)...);
    }

    const auto current_version = ComputeHash(args...);

    if (current_version == current_itr->second.version)
    {
      return element_ref{ResourceStatus::kExisted, handle, std::addressof(current_itr->second.value)};
    }

    return replace_at_position(current_itr, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> find_or_create(handle_type handle, CreateArgTs&&... args)
  {
    // If hint handle is null, create a new resource, otherwise attempt replacement
    return handle.isNull() ? create(std::forward<CreateArgTs>(args)...)
                           : find_or_replace(handle, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> emplace_with_hint(handle_type handle, CreateArgTs&&... args)
  {
    // If hint handle is null, create a new resource, otherwise attempt creation at handle
    return handle.isNull() ? create(std::forward<CreateArgTs>(args)...)
                           : create_at_handle(handle, std::forward<CreateArgTs>(args)...);
  }

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> insert(handle_type handle, version_type version, value_type&& value)
  {
    if (handle.isNull())
    {
      return make_unexpected(error_type::kInvalidHandle);
    }

    // Add it to the cache
    const auto [itr, added] = handle_to_value_cache_.emplace(
      std::piecewise_construct, std::forward_as_tuple(handle), std::forward_as_tuple(version, std::move(value)));

    if (added)
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      return element_ref{ResourceStatus::kReplaced, itr->first, std::addressof(itr->second.value)};
    }

    // Element was a duplicate
    return make_unexpected(error_type::kElementAlreadyExists);
  }

  [[nodiscard]] const value_type* get_if(handle_type handle) const
  {
    if (auto itr = handle_to_value_cache_.find(handle); itr != std::end(handle_to_value_cache_))
    {
      return std::addressof(itr->second.value);
    }
    return nullptr;
  }

  [[nodiscard]] element_ref operator()(handle_type handle) const { return find(handle); }

  [[nodiscard]] element_ref find(handle_type handle) const
  {
    const auto value_ptr = get_if(handle);
    return {(value_ptr == nullptr) ? ResourceStatus::kInvalid : ResourceStatus::kExisted, handle, value_ptr};
  }

  [[nodiscard]] const bool exists(handle_type handle) const { return handle_to_value_cache_.count(handle) != 0; }

  void remove(handle_type handle)
  {
    if (auto itr = handle_to_value_cache_.find(handle); itr != std::end(handle_to_value_cache_))
    {
      this->derived().when_removed(itr->first, std::addressof(itr->second.value));
      handle_to_value_cache_.erase(itr);
    }
  }

  [[nodiscard]] const auto& cache() const { return handle_to_value_cache_; }

  [[nodiscard]] const auto begin() const { return std::begin(handle_to_value_cache_); }

  [[nodiscard]] const auto end() const { return std::end(handle_to_value_cache_); }

  [[nodiscard]] std::size_t size() const { return handle_to_value_cache_.size(); }

  template <bool U = (dependencies::size() > 0)>
  [[nodiscard]] std::enable_if_t<U, expected<void, error_type>> refresh(dependencies deps)
  {
    for (auto& [handle, element] : handle_to_value_cache_)
    {
      if (auto ok_or_error = this->derived().reload(deps, element.value); !ok_or_error.has_value())
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <bool U = (dependencies::size() > 0)>
  [[nodiscard]] std::enable_if_t<!U, expected<void, error_type>> refresh()
  {
    for (auto& [handle, element] : handle_to_value_cache_)
    {
      if (auto ok_or_error = this->derived().reload(element.value); !ok_or_error.has_value())
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <bool U = (dependencies::size() > 0)>
  [[nodiscard]] std::enable_if_t<U, expected<void, error_type>> relinquish(dependencies deps)
  {
    for (auto& [handle, element] : handle_to_value_cache_)
    {
      if (auto ok_or_error = this->derived().unload(deps, element.value); !ok_or_error.has_value())
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <bool U = (dependencies::size() > 0)>
  [[nodiscard]] std::enable_if_t<!U, expected<void, error_type>> relinquish()
  {
    for (auto& [handle, element] : handle_to_value_cache_)
    {
      if (auto ok_or_error = this->derived().unload(element.value); !ok_or_error.has_value())
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <typename UpdateFn> void update_if_exists(handle_type handle, UpdateFn update)
  {
    const auto handle_to_value_itr = handle_to_value_cache_.find(handle);
    if (handle_to_value_itr != handle_to_value_cache_.end())
    {
      update(handle_to_value_itr->second.value);
    }
  }

  void swap(ResourceCache& other)
  {
    std::swap(this->handle_lower_bound_, other.handle_lower_bound_);
    std::swap(this->handle_to_value_cache_, other.handle_to_value_cache_);
  }

  ResourceCache() = default;

  ResourceCache(ResourceCache&& other) { this->swap(other); }

  ResourceCache& operator=(ResourceCache&& other)
  {
    this->swap(other);
    return *this;
  }

private:
  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> create_at_handle(handle_type handle, CreateArgTs&&... args)
  {
    const auto current_version = ComputeHash(args...);

    // Create a new element
    auto value_or_error = this->derived().generate(std::forward<CreateArgTs>(args)...);
    if (!value_or_error.has_value())
    {
      return make_unexpected(value_or_error.error());
    }

    // Add it to the cache
    const auto [itr, added] = handle_to_value_cache_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(handle),
      std::forward_as_tuple(current_version, std::move(value_or_error).value()));

    // Update handle lower bound for next element creates
    if (added)
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      this->derived().when_created(itr->first, std::addressof(itr->second.value));
      return element_ref{ResourceStatus::kCreated, itr->first, std::addressof(itr->second.value)};
    }
    return make_unexpected(error_type::kInvalidHandle);
  }

  template <typename Iterator, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> replace_at_position(Iterator itr, CreateArgTs&&... args)
  {
    const auto current_version = ComputeHash(args...);

    // Create a new element
    auto value_or_error = this->derived().generate(std::forward<CreateArgTs>(args)...);
    if (!value_or_error.has_value())
    {
      return make_unexpected(value_or_error.error());
    }

    // Replace current value
    itr->second.version = current_version;
    itr->second.value = std::move(value_or_error).value();
    this->derived().when_created(itr->first, std::addressof(itr->second.value));
    return element_ref{ResourceStatus::kReplaced, itr->first, std::addressof(itr->second.value)};
  }

  template <typename... Ts> [[nodiscard]] static expected<void, error_type> reload([[maybe_unused]] Ts&&... _)
  {
    return {};
  }

  template <typename... Ts> [[nodiscard]] static expected<void, error_type> unload([[maybe_unused]] Ts&&... _)
  {
    return {};
  }

  [[nodiscard]] static handle_type next_unique_id([[maybe_unused]] const CacheMap& map, handle_type lower_bound)
  {
    ++lower_bound;
    return lower_bound;
  }

  ResourceCache(const ResourceCache&) = delete;
  ResourceCache& operator=(const ResourceCache&) = delete;

protected:
  /// Last used resource handle
  handle_type handle_lower_bound_ = handle_type::null();

  /// Map of {resource_handle, resource_value} objects
  CacheMap handle_to_value_cache_;

private:
  static void when_created([[maybe_unused]] handle_type h, [[maybe_unused]] const value_type* value) {}
  static void when_removed([[maybe_unused]] handle_type h, [[maybe_unused]] const value_type* value) {}
};

template <typename T> struct is_resource_cache : std::is_base_of<ResourceCache<T>, T>
{};

template <typename R> constexpr bool is_resource_cache_v = is_resource_cache<std::remove_const_t<R>>::value;


}  // namespace sde
