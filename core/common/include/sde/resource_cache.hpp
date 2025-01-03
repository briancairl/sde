/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
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


template <typename ResourceCacheT>
struct ResourceCacheHasDependencies
    : std::integral_constant<
        bool,
        !std::is_same_v<typename ResourceCacheTraits<ResourceCacheT>::dependencies, ResourceDependencies<>>>
{};

template <typename ResourceCacheT>
static constexpr bool resource_cache_has_dependencies_v = ResourceCacheHasDependencies<ResourceCacheT>::value;


template <typename ResourceCacheT> class ResourceCache : public crtp_base<ResourceCache<ResourceCacheT>>
{
public:
  using dependencies = expand_resource_dependencies_t<typename ResourceCacheTraits<ResourceCacheT>::dependencies>;
  using type_info = ResourceCacheTraits<ResourceCacheT>;
  using error_type = typename type_info::error_type;
  using handle_type = typename type_info::handle_type;
  using value_type = typename type_info::value_type;
  using version_type = Hash;

  static_assert(std::is_enum_v<error_type>, "'error_type' must be an enum type");
  static_assert(is_resource_handle_v<handle_type>, "'handle_type' must be a ResourceHandle<...> type");
  static_assert(is_resource_v<value_type>, "'value_type' must be a Resource<...> type");

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
  using ElementMap = sde::unordered_map<
    handle_type,
    element_storage,
    handle_type_hash,
    std::equal_to<handle_type>
  >;
  // clang-format on

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> create(dependencies deps, CreateArgTs&&... args)
  {
    const auto handle = this->derived().next_unique_id(handle_to_value_cache_, handle_lower_bound_);
    auto element_or_error = this->create_at_handle(handle, deps, std::forward<CreateArgTs>(args)...);
    if (element_or_error.has_value())
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      return std::move(element_or_error).value();
    }
    return make_unexpected(element_or_error.error());
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type>
  find_and_replace_or_create(HandleT&& handle_or, dependencies deps, CreateArgTs&&... args)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
    if (handle.isNull())
    {
      return create(deps, std::forward<CreateArgTs>(args)...);
    }

    const auto itr = handle_to_value_cache_.find(handle);
    if (itr == handle_to_value_cache_.end())
    {
      return create_at_handle(handle, deps, std::forward<CreateArgTs>(args)...);
    }

    if (!on_removal(deps, itr->first, std::addressof(itr->second.value)))
    {
      return make_unexpected(error_type::kInvalidHandle);
    }
    return replace_at_position(itr, deps, std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type>
  find_or_replace(HandleT&& handle_or, dependencies deps, CreateArgTs&&... args)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));

    const auto itr = handle_to_value_cache_.find(handle);

    if (itr == handle_to_value_cache_.end())
    {
      return create_at_handle(handle, deps, std::forward<CreateArgTs>(args)...);
    }

    if (const auto current_version = ComputeHash(args...); current_version == itr->second.version)
    {
      return element_ref{ResourceStatus::kExisted, handle, std::addressof(itr->second.value)};
    }

    return replace_at_position(itr, deps, std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type>
  find_or_create(HandleT&& handle_or, dependencies deps, CreateArgTs&&... args)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));

    // If hint handle is null, create a new resource, otherwise attempt replacement
    return handle.isNull() ? create(deps, std::forward<CreateArgTs>(args)...)
                           : find_or_replace(handle, deps, std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type>
  emplace_with_hint(HandleT&& handle_or, dependencies deps, CreateArgTs&&... args)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));

    // If hint handle is null, create a new resource, otherwise attempt creation at handle
    return handle.isNull() ? create(deps, std::forward<CreateArgTs>(args)...)
                           : create_at_handle(handle, deps, std::forward<CreateArgTs>(args)...);
  }

  template <typename HandleT, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type> insert(HandleT&& handle_or, version_type version, value_type&& value)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
    if (handle.isNull())
    {
      return make_unexpected(error_type::kInvalidHandle);
    }

    // clang-format off
    // Add it to the cache
    const auto [itr, added] = handle_to_value_cache_.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(handle),
      std::forward_as_tuple(version, std::move(value))
    );
    // clang-format on

    if (added)
    {
      handle_lower_bound_ = std::max(handle_lower_bound_, handle);
      return element_ref{ResourceStatus::kReplaced, itr->first, std::addressof(itr->second.value)};
    }

    // Element was a duplicate
    return make_unexpected(error_type::kElementAlreadyExists);
  }

  template <typename HandleT> [[nodiscard]] element_ref operator()(HandleT&& handle_or) const
  {
    return find(std::forward<HandleT>(handle_or));
  }

  template <typename HandleT> [[nodiscard]] element_ref find(HandleT&& handle_or) const
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
    const auto value_ptr = get_if(handle);
    return {(value_ptr == nullptr) ? ResourceStatus::kInvalid : ResourceStatus::kExisted, handle, value_ptr};
  }

  template <typename HandleT> [[nodiscard]] bool exists(HandleT&& handle_or) const
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
    return handle_to_value_cache_.count(handle) != 0;
  }

  [[nodiscard]] bool empty() const { return handle_to_value_cache_.empty(); }

  [[nodiscard]] const auto& cache() const { return handle_to_value_cache_; }

  [[nodiscard]] const auto begin() const { return std::begin(handle_to_value_cache_); }

  [[nodiscard]] const auto end() const { return std::end(handle_to_value_cache_); }

  [[nodiscard]] std::size_t size() const { return handle_to_value_cache_.size(); }

  [[nodiscard]] expected<void, error_type> refresh(dependencies deps)
  {
    for (auto& [handle, element] : handle_to_value_cache_)
    {
      if (auto ok_or_error = this->derived().reload(deps, element.value); !ok_or_error.has_value())
      {
        return ok_or_error;
      }
      if (!on_creation(deps, handle, std::addressof(element.value)))
      {
        return make_unexpected(error_type::kElementCreationFailure);
      }
    }
    return {};
  }

  [[nodiscard]] expected<void, error_type> relinquish(dependencies deps)
  {
    for (auto& [handle, element] : handle_to_value_cache_)
    {
      if (!on_removal(deps, handle, std::addressof(element.value)))
      {
        return make_unexpected(error_type::kElementRemovalFailure);
      }
      if (auto ok_or_error = this->derived().unload(deps, element.value); !ok_or_error.has_value())
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <typename HandleT, typename UpdateFn> void update_if_exists(HandleT&& handle_or, UpdateFn update)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
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

  template <typename HandleT> expected<void, error_type> remove(HandleT&& handle_or, dependencies deps)
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
    const auto itr = handle_to_value_cache_.find(handle);
    if (itr == std::end(handle_to_value_cache_))
    {
      return make_unexpected(error_type::kInvalidHandle);
    }
    if (!on_removal(deps, itr->first, std::addressof(itr->second.value)))
    {
      return make_unexpected(error_type::kElementRemovalFailure);
    }
    handle_to_value_cache_.erase(itr);
    return {};
  }

  std::size_t prune(dependencies deps)
  {
    const std::size_t initial_size = handle_to_value_cache_.size();
    for (auto itr = std::begin(handle_to_value_cache_); itr != std::end(handle_to_value_cache_); /*empty*/)
    {
      if ((itr->second.usage_count == 0) and on_removal(deps, itr->first, std::addressof(itr->second.value)))
      {
        itr = handle_to_value_cache_.erase(itr);
      }
      else
      {
        ++itr;
      }
    }
    return initial_size - handle_to_value_cache_.size();
  }

  bool clear(dependencies deps)
  {
    for (auto& [handle, storage] : handle_to_value_cache_)
    {
      if (!on_removal(deps, handle, std::addressof(storage.value)))
      {
        return false;
      }
    }
    handle_to_value_cache_.clear();
    handle_lower_bound_ = handle_type::null();
    return true;
  }

  ResourceCache() = default;

  ResourceCache(ResourceCache&& other) { this->swap(other); }

  ResourceCache& operator=(ResourceCache&& other)
  {
    this->swap(other);
    return *this;
  }

protected:
  constexpr static handle_type to_handle(handle_type handle) { return handle; }

  /// Last used resource handle
  handle_type handle_lower_bound_ = handle_type::null();

  /// Map of {resource_handle, resource_value} objects
  ElementMap handle_to_value_cache_;

private:
  ResourceCache(const ResourceCache&) = delete;
  ResourceCache& operator=(const ResourceCache&) = delete;

  template <typename HandleT> [[nodiscard]] const value_type* get_if(HandleT&& handle_or) const
  {
    const auto handle = this->derived().to_handle(std::forward<HandleT>(handle_or));
    if (auto itr = handle_to_value_cache_.find(handle); itr != std::end(handle_to_value_cache_))
    {
      return std::addressof(itr->second.value);
    }
    return nullptr;
  }

  template <typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type>
  create_at_handle(handle_type handle, dependencies deps, CreateArgTs&&... args)
  {
    if (handle.isNull())
    {
      return make_unexpected(error_type::kInvalidHandle);
    }
    const auto current_version = ComputeHash(args...);

    // Create a new element
    auto value_or_error = this->derived().generate(deps, std::forward<CreateArgTs>(args)...);
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
      if (on_creation(deps, itr->first, std::addressof(itr->second.value)))
      {
        return element_ref{ResourceStatus::kCreated, itr->first, std::addressof(itr->second.value)};
      }
      else
      {
        return make_unexpected(error_type::kElementCreationFailure);
      }
    }
    return make_unexpected(error_type::kInvalidHandle);
  }

  template <typename Iterator, typename... CreateArgTs>
  [[nodiscard]] expected<element_ref, error_type>
  replace_at_position(Iterator itr, dependencies deps, CreateArgTs&&... args)
  {
    const auto current_version = ComputeHash(args...);

    // Create a new element
    auto value_or_error = this->derived().generate(deps, std::forward<CreateArgTs>(args)...);
    if (!value_or_error.has_value())
    {
      return make_unexpected(value_or_error.error());
    }

    // Replace current value
    itr->second.version = current_version;
    itr->second.value = std::move(value_or_error).value();

    if (on_creation(deps, itr->first, std::addressof(itr->second.value)))
    {
      return element_ref{ResourceStatus::kReplaced, itr->first, std::addressof(itr->second.value)};
    }
    else
    {
      return make_unexpected(error_type::kElementCreationFailure);
    }
    return make_unexpected(error_type::kInvalidHandle);
  }

  template <typename... Ts> [[nodiscard]] static expected<void, error_type> reload([[maybe_unused]] Ts&&... _)
  {
    return {};
  }

  template <typename... Ts> [[nodiscard]] static expected<void, error_type> unload([[maybe_unused]] Ts&&... _)
  {
    return {};
  }

  [[nodiscard]] static handle_type next_unique_id([[maybe_unused]] const ElementMap& map, handle_type lower_bound)
  {
    ++lower_bound;
    return lower_bound;
  }

  bool on_creation(dependencies deps, handle_type h, value_type* value)
  {
    this->derived().when_created(deps, h, value);
    return true;
  }

  bool on_removal(dependencies deps, handle_type h, value_type* value)
  {
    this->derived().when_removed(deps, h, value);
    return true;
  }

  static constexpr bool when_created(
    [[maybe_unused]] dependencies deps,
    [[maybe_unused]] handle_type h,
    [[maybe_unused]] const value_type* value)
  {
    return true;
  }

  static constexpr bool when_removed(
    [[maybe_unused]] dependencies deps,
    [[maybe_unused]] handle_type h,
    [[maybe_unused]] const value_type* value)
  {
    return true;
  }
};

template <typename T> struct IsResourceCache : std::is_base_of<ResourceCache<T>, T>
{};

template <typename R> constexpr bool is_resource_cache_v = IsResourceCache<std::remove_const_t<R>>::value;

template <typename T> struct IsResourceCacheLike : IsResourceCache<T>
{};

template <typename R> constexpr bool is_resource_cache_like_v = IsResourceCacheLike<std::remove_const_t<R>>::value;

}  // namespace sde

#define SDE_RESOURCE_CACHE_ERROR_ENUMS                                                                                 \
  kInvalidHandle, kElementAlreadyExists, kElementCreationFailure, kElementRemovalFailure, kElementNotInUse

#define SDE_OS_ENUM_CASES_FOR_RESOURCE_CACHE_ERRORS(error_type)                                                        \
  SDE_OS_ENUM_CASE(error_type::kInvalidHandle)                                                                         \
  SDE_OS_ENUM_CASE(error_type::kElementAlreadyExists)                                                                  \
  SDE_OS_ENUM_CASE(error_type::kElementCreationFailure)                                                                \
  SDE_OS_ENUM_CASE(error_type::kElementRemovalFailure)                                                                 \
  SDE_OS_ENUM_CASE(error_type::kElementNotInUse)
