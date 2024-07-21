/**
 * @copyright 2024-present Brian Cairl
 *
 * @file resource_io.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>

// SDE
#include "sde/resource.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive, typename T> struct serialize<Archive, Field<T>>
{
  void operator()(Archive& ar, Field<T>& field) const { ar& named{field.name, *field}; }
};

template <typename Archive, typename T> struct serialize<Archive, _Stub<T>>
{
  void operator()(Archive& ar, _Stub<T>& field) const {}
};

template <typename Archive, typename ResourceT> struct save<Archive, Resource<ResourceT>>
{
  void operator()(Archive& ar, const Resource<ResourceT>& resource) const
  {
    std::apply([&ar](auto&&... field) { [[maybe_unused]] auto _ = ((ar << field, 0) + ...); }, resource.fields());
  }
};

template <typename Archive, typename ResourceT> struct load<Archive, Resource<ResourceT>>
{
  void operator()(Archive& ar, Resource<ResourceT>& resource) const
  {
    std::apply([&ar](auto&&... field) { [[maybe_unused]] auto _ = ((ar >> field, 0) + ...); }, resource.fields());
  }
};

}  // namespace sde::serial
