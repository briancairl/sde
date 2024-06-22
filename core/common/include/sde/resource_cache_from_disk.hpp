/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache_from_disk.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <cstdint>
#include <utility>
#include <variant>
#include <vector>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/resource_cache.hpp"

namespace sde
{

template <typename ResourceCacheT, typename LoadFromDiskT> class ResourceCacheFromDisk;

enum class ResourceLoadError
{
  kResourceUnavailable,
  kResourceAlreadyLoaded,
};

template <typename ResourceCacheT, typename LoadFromDiskT> class ResourceCacheFromDisk
{
  static_assert(is_resource_cache_v<ResourceCacheT>);

public:
  using error_type = std::variant<ResourceLoadError, typename ResourceCacheTypes<ResourceCacheT>::error_type>;
  using handle_type = typename ResourceCacheTypes<ResourceCacheT>::handle_type;
  using value_type = typename ResourceCacheTypes<ResourceCacheT>::value_type;
  using PathList = std::vector<std::pair<handle_type, asset::path>>;

  explicit ResourceCacheFromDisk(ResourceCacheT& cache, LoadFromDiskT load_fn = LoadFromDiskT{}) :
      cache_{std::addressof(cache)}, load_fn_{std::move(load_fn)}
  {}

  bool isLoaded(const asset::path& query_path) const
  {
    return std::any_of(
      std::begin(path_list_), std::end(path_list_), [&query_path](const auto& handle_and_path) -> bool {
        return query_path == std::get<1>(handle_and_path);
      });
  }

  expected<element_t<ResourceCacheT>, error_type> create(const asset::path& path)
  {
    if (isLoaded(path))
    {
      return make_unexpected(ResourceLoadError::kResourceAlreadyLoaded);
    }

    if (!asset::exists(path))
    {
      return make_unexpected(ResourceLoadError::kResourceUnavailable);
    }

    auto element_or_error = load_fn_(*cache_, path);
    if (element_or_error.has_value())
    {
      path_list_.emplace_back(element_or_error->handle, path);
      return std::move(element_or_error).value();
    }
    return make_unexpected(element_or_error.error());
  }

  void remove(const handle_type& handle) const
  {
    const auto itr =
      std::find_if(std::begin(path_list_), std::end(path_list_), [&handle](const auto& handle_and_path) -> bool {
        return handle == std::get<0>(handle_and_path);
      });
    if (itr != std::end(path_list_))
    {
      path_list_.erase(itr);
      cache_->remove(handle);
    }
  }

  const PathList& paths() const { return path_list_; }

private:
  PathList path_list_;
  ResourceCacheT* cache_;
  LoadFromDiskT load_fn_;
};

template <typename ResourceCacheT, typename LoadFromDiskT>
ResourceCacheFromDisk<ResourceCacheT, LoadFromDiskT> from_disk(ResourceCacheT& cache, LoadFromDiskT load_fn)
{
  return ResourceCacheFromDisk<ResourceCacheT, LoadFromDiskT>{cache, std::move(load_fn)};
}

}  // namespace sde
