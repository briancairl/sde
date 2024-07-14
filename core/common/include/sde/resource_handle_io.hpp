/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_handle_io.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive, typename T> struct save<Archive, ResourceHandle<T>>
{
  void operator()(Archive& ar, const ResourceHandle<T>& handle) const { ar << named{"id", handle.id()}; }
};

template <typename Archive, typename T> struct load<Archive, ResourceHandle<T>>
{
  void operator()(Archive& ar, ResourceHandle<T>& handle) const
  {
    auto id = handle.id();
    ar >> named{"id", id};
    handle = id;
  }
};

}  // namespace sde::serial
