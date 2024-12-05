/**
 * @copyright 2024-present Brian Cairl
 *
 * @file library_fwd.hpp
 */
#pragma once

// SDE
#include "sde/resource_cache_traits.hpp"

namespace sde::game
{
enum class LibraryError;
struct LibraryHandle;
struct LibraryData;
class LibraryCache;
}  // namespace sde::game

namespace sde
{
template <> struct ResourceCacheTraits<game::LibraryCache>
{
  using error_type = game::LibraryError;
  using handle_type = game::LibraryHandle;
  using value_type = game::LibraryData;
  using dependencies = no_dependencies;
};
}  // namespace sde