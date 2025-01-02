/**
 * @copyright 2024-present Brian Cairl
 *
 * @file unordered_map.hpp
 */
#pragma once

// C++ Standard Library
#include <unordered_map>

// SDE
#include "sde/expected.hpp"
#include "sde/format.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/structure.hpp"

namespace sde::serial
{

template <typename OArchiveT, typename KeyT, typename ValueT, typename HashT, typename EqualT, typename Allocator>
struct save<OArchiveT, std::unordered_map<KeyT, ValueT, HashT, EqualT, Allocator>>
{
  void operator()(OArchiveT& oar, const std::unordered_map<KeyT, ValueT, HashT, EqualT, Allocator>& umap)
  {
    oar << named{"element_count", umap.size()};
    std::size_t i = 0;
    for (const auto& [key, value] : umap)
    {
      oar << named{
        format("%lu", i++),
        structure{
          named{"key", key},
          named{"value", value},
        }};
    }
  }
};

template <typename IArchiveT, typename KeyT, typename ValueT, typename HashT, typename EqualT, typename Allocator>
struct load<IArchiveT, std::unordered_map<KeyT, ValueT, HashT, EqualT, Allocator>>
{
  expected<void, iarchive_error>
  operator()(IArchiveT& iar, std::unordered_map<KeyT, ValueT, HashT, EqualT, Allocator>& umap)
  {
    std::size_t element_count{0};
    iar >> named{"element_count", element_count};
    umap.reserve(element_count);
    for (std::size_t i = 0; i < element_count; ++i)
    {
      KeyT key;
      ValueT value;

      auto kv_pair = structure{named{"key", key}, named{"value", value}};

      iar >> named{format("%lu", i), kv_pair};

      if (auto [_, added] = umap.emplace(std::move(key), std::move(value)); !added)
      {
        return make_unexpected(iarchive_error::kLoadFailure);
      }
    }
    return {};
  }
};

}  // namespace sde::serial
