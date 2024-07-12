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
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive, typename ResourceT> struct save<Archive, Field<T>>
{
  void operator()(Archive& ar, const Field<T>& field) const { ar << named{field.name, field.value}; }
};

template <typename Archive, typename T> struct load<Archive, Field<T>>
{
  void operator()(Archive& ar, Field<T>& field) const { ar >> named{field.name, field.value}; }
};

template <typename Archive, typename ResourceT> struct save<Archive, FieldNotSerialized<T>>
{
  void operator()(Archive& ar, const FieldNotSerialized<T>& field) const {}
};

template <typename Archive, typename T> struct load<Archive, FieldNotSerialized<T>>
{
  void operator()(Archive& ar, FieldNotSerialized<T>& field) const {}
};

template <typename Archive, typename ResourceT> struct save<Archive, Resource<ResourceT>>
{
  void operator()(Archive& ar, const Resource<ResourceT>& resource) const
  {
    std::apply([&ar](const auto&... field) { ((ar << field, 0) + ...); }, resource.fields());
  }
};

template <typename Archive, typename ResourceT> struct load<Archive, Resource<ResourceT>>
{
  void operator()(Archive& ar, Resource<ResourceT>& resource) const
  {
    std::apply([&ar](const auto&... field) { ((ar >> field, 0) + ...); }, resource.fields());
  }
};

}  // namespace sde::serial
