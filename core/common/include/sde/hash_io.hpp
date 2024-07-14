/**
 * @copyright 2024-present Brian Cairl
 *
 * @file hash_io.hpp
 */
#pragma once

// SDE
#include "sde/hash.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive> struct serialize<Archive, Hash>
{
  void operator()(Archive& ar, Hash& hash) const { ar& named{"value", hash.value}; }
};

}  // namespace sde::serial
