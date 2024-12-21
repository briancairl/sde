/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component_decl.hpp
 */
#pragma once

// C++ Standard Library
#include <string_view>

// SDE
#include "sde/type_name.hpp"

template <typename T> struct ComponentName
{
  static constexpr auto value = ::sde::type_name<T>();
};

#define SDE_COMPONENT_RENAME(ComponentT, Name)                                                                         \
  template <> struct ComponentName<ComponentT>                                                                         \
  {                                                                                                                    \
    static constexpr auto value = std::string_view{Name};                                                              \
  }
