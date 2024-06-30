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

template <typename Archive, typename T, typename IdentifierT, IdentifierT kNullValue>
struct save<Archive, ResourceHandle<T, IdentifierT, kNullValue>>
{
  void operator()(Archive& ar, const ResourceHandle<T, IdentifierT, kNullValue>& handle) const
  {
    ar << named{"id", handle.id()};
  }
};

template <typename Archive, typename T, typename IdentifierT, IdentifierT kNullValue>
struct load<Archive, ResourceHandle<T, IdentifierT, kNullValue>>
{
  void operator()(Archive& ar, ResourceHandle<T, IdentifierT, kNullValue>& handle) const
  {
    IdentifierT id{};
    ar >> named{"id", id};
    handle = ResourceHandle<T, IdentifierT, kNullValue>{id};
  }
};

}  // namespace sde::serial
