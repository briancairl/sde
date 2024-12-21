/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component_runtime.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>

// Entt
#include <entt/entt.hpp>

// SDE
#include "sde/dl/export.hpp"
#include "sde/game/archive.hpp"
#include "sde/game/registry.hpp"
#include "sde/type_name.hpp"

namespace sde::game
{

template <typename ComponentT> const char* component_name_impl()
{
  static constexpr auto kTypeName = type_name<ComponentT>();
  static char type_name_buffer[kTypeName.size() + 2] = {};
  if (type_name_buffer[0] == 0)
  {
    auto* end_itr = std::copy(std::begin(kTypeName), std::end(kTypeName), std::begin(type_name_buffer));
    (*end_itr) = '\0';
  }
  return type_name_buffer;
}

template <typename ComponentT> void component_load_impl(sde::game::IArchive& ar, EntityID e, Registry& registry)
{
  if constexpr (std::is_void_v<decltype(registry.template emplace<ComponentT>(e))>)
  {
    registry.template emplace<ComponentT>(e);
  }
  else
  {
    // ar >> registry.template emplace<ComponentT>(e);
  }
}

template <typename ComponentT> void component_save_impl(sde::game::OArchive& ar, EntityID e, const Registry& registry)
{
  if constexpr (!std::is_void_v<decltype(registry.template get<ComponentT>(e))>)
  {
    // ar << registry.template get<ComponentT>(e);
  }
}

}  // namespace sde::game

#define SDE_COMPONENT__REGISTER_NAME(Name, ComponentT)                                                                 \
  SDE_EXPORT const char* Name##_name() { return ::sde::game::component_name_impl<ComponentT>(); }


#define SDE_COMPONENT__REGISTER_LOAD(Name, ComponentT)                                                                 \
  SDE_EXPORT void Name##_on_load(void* iarchive, void* entity, void* registry)                                         \
  {                                                                                                                    \
    ::sde::game::component_load_impl<ComponentT>(                                                                      \
      *reinterpret_cast<::sde::game::IArchive*>(iarchive),                                                             \
      *reinterpret_cast<::sde::game::EntityID*>(entity),                                                               \
      *reinterpret_cast<::sde::game::Registry*>(registry));                                                            \
  }

#define SDE_COMPONENT__REGISTER_SAVE(Name, ComponentT)                                                                 \
  SDE_EXPORT void Name##_on_save(void* oarchive, void* entity, const void* registry)                                   \
  {                                                                                                                    \
    ::sde::game::component_save_impl<ComponentT>(                                                                      \
      *reinterpret_cast<::sde::game::OArchive*>(oarchive),                                                             \
      *reinterpret_cast<::sde::game::EntityID*>(entity),                                                               \
      *reinterpret_cast<const ::sde::game::Registry*>(registry));                                                      \
  }


#define SDE_COMPONENT__REGISTER(Name, ComponentT)                                                                      \
  SDE_COMPONENT__REGISTER_NAME(Name, ComponentT)                                                                       \
  SDE_COMPONENT__REGISTER_LOAD(Name, ComponentT)                                                                       \
  SDE_COMPONENT__REGISTER_SAVE(Name, ComponentT)
