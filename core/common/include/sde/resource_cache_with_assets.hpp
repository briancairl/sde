/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache_from_disk.hpp
 */
#pragma once

// C++ Standard Library
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <utility>
#include <variant>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/resource_cache.hpp"

namespace sde
{

template <typename ResourceCacheT> struct LoadAssetPassthrough
{
  typename ResourceCacheT::result_type operator()(ResourceCacheT& cache, const asset::path& path) const
  {
    return cache.create(path);
  }
};

template <typename ResourceCacheT, typename LoadAssetT = LoadAssetPassthrough<ResourceCacheT>>
class ResourceCacheWithAssets : public ResourceCacheT
{
  static_assert(is_resource_cache_v<ResourceCacheT>);

public:
  using error_type = typename ResourceCacheTypes<ResourceCacheT>::error_type;
  using handle_type = typename ResourceCacheTypes<ResourceCacheT>::handle_type;
  using value_type = typename ResourceCacheTypes<ResourceCacheT>::value_type;
  using AssetToHandle = std::unordered_map<asset::path, handle_type>;
  using HandleToAsset = std::unordered_map<handle_type, asset::path, ResourceHandleHash>;

  explicit ResourceCacheWithAssets(LoadAssetT load_asset = LoadAssetT{}) :
      ResourceCacheT{}, load_asset_{std::move(load_asset)}
  {}

  handle_type find(const asset::path& query_path) const
  {
    const auto path_and_handle_itr = asset_to_handle_.find(query_path);
    return (path_and_handle_itr == std::end(asset_to_handle_)) ? handle_type::null() : path_and_handle_itr->second;
  }

  bool isLoaded(const asset::path& query_path) const { return find(query_path).isValid(); }

  template <typename... LoadAssetArgTs>
  expected<element_t<ResourceCacheT>, error_type> load(const asset::path& path, LoadAssetArgTs&&... args)
  {
    if (auto existing_handle = this->find(path); existing_handle)
    {
      return ResourceCacheT::find(existing_handle);
    }

    if (!asset::exists(path))
    {
      return make_unexpected(error_type::kAssetNotFound);
    }

    auto element_or_error =
      load_asset_(static_cast<ResourceCacheT&>(*this), path, std::forward<LoadAssetArgTs>(args)...);
    if (element_or_error.has_value())
    {
      asset_to_handle_.emplace(path, element_or_error->handle);
      handle_to_asset_.emplace(element_or_error->handle, path);
      return std::move(element_or_error).value();
    }
    return make_unexpected(element_or_error.error());
  }

  void remove(const handle_type& handle) const
  {
    if (const auto itr = handle_to_asset_.find(handle); itr != std::end(handle_to_asset_))
    {
      asset_to_handle_.erase(itr->second);
      handle_to_asset_.erase(itr);
    }
    ResourceCacheT::remove(handle);
  }

private:
  using ResourceCacheT::remove;

  HandleToAsset handle_to_asset_;
  AssetToHandle asset_to_handle_;
  LoadAssetT load_asset_;
};

}  // namespace sde
