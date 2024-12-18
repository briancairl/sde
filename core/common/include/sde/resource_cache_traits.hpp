#pragma once

// SDE
#include "sde/resource_dependencies.hpp"

namespace sde
{
template <typename ResourceCacheT> struct ResourceCacheTraits
{
  using error_type = void;
  using handle_type = void;
  using value_type = void;
  using dependencies = no_dependencies;
};
}  // namespace sde