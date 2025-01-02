/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache_io.hpp
 */
#pragma once

// SDE
#include "sde/expected.hpp"
#include "sde/format.hpp"
#include "sde/hash_io.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive, typename CacheT> struct save<Archive, ResourceCache<CacheT>>
{
  void operator()(Archive& oar, const ResourceCache<CacheT>& cache) const
  {
    oar << named{"element_count", cache.size()};
    std::size_t element_idx = 0;
    for (const auto& [handle, element] : cache)
    {
      oar << named{
        format("element[%lu]", element_idx++),
        structure{named{"handle", handle}, named{"version", element.version}, named{"value", element.value}}};
    }
  }
};

template <typename Archive, typename CacheT> struct load<Archive, ResourceCache<CacheT>>
{
  expected<void, iarchive_error> operator()(Archive& iar, ResourceCache<CacheT>& cache) const
  {
    using version_type = typename ResourceCache<CacheT>::version_type;
    using handle_type = typename ResourceCacheTraits<CacheT>::handle_type;
    using value_type = typename ResourceCacheTraits<CacheT>::value_type;

    std::size_t element_count{0};
    iar >> named{"element_count", element_count};
    for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
    {
      handle_type handle;
      version_type version;
      value_type value;

      auto s = structure{named{"handle", handle}, named{"version", version}, named{"value", value}};

      iar >> named{format("element[%lu]", element_idx), s};

      if (auto ok_or_error = cache.insert(handle, version, std::move(value)); !ok_or_error)
      {
        return make_unexpected(iarchive_error::kLoadFailure);
      }
    }
    return {};
  }
};

}  // namespace sde::serial
