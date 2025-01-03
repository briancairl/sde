/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_cache_io.hpp
 */
#pragma once

// SDE
#include "sde/hash_io.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive, typename CacheT> struct save<Archive, ResourceCache<CacheT>>
{
  void operator()(Archive& ar, const ResourceCache<CacheT>& cache) const
  {
    ar << named{"element_count", cache.size()};
    for (const auto& [handle, element] : cache)
    {
      ar << named{"handle", handle};
      ar << named{"version", element.version};
      ar << named{"value", element.value};
    }
  }
};

template <typename Archive, typename CacheT> struct load<Archive, ResourceCache<CacheT>>
{
  void operator()(Archive& ar, ResourceCache<CacheT>& cache) const
  {
    using version_type = typename ResourceCache<CacheT>::version_type;
    using handle_type = typename ResourceCacheTraits<CacheT>::handle_type;
    using value_type = typename ResourceCacheTraits<CacheT>::value_type;

    std::size_t element_count{0};
    ar >> named{"element_count", element_count};
    for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
    {
      handle_type handle;
      ar >> named{"handle", handle};
      version_type version;
      ar >> named{"version", version};
      value_type value;
      ar >> named{"value", value};
      auto _ = cache.insert(handle, version, std::move(value));
      (void)_;
    }
  }
};

}  // namespace sde::serial
