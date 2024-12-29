/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_runtime.hpp
 */
#pragma once

// C++ Standard Library
#include <optional>
#include <string_view>

// SDE
#include "sde/app_properties.hpp"
#include "sde/dl/export.hpp"
#include "sde/expected.hpp"
#include "sde/game/archive.hpp"
#include "sde/game/entity.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_runtime_fwd.hpp"
#include "sde/geometry_io.hpp"
#include "sde/logging.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/optional.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/serial/std/unordered_map.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time.hpp"
#include "sde/time_io.hpp"


struct native_script_data : private sde::game::native_script_header
{
  constexpr bool initialized() const { return initialization_time_point.has_value(); }
  constexpr std::string_view name() const { return name; }
  constexpr script_id_t uid() const { return uid; }
  constexpr script_version_t version() const { return version; }
};


#define SDE_NATIVE_SCRIPT__REGISTER_CREATE(ScriptDataT)                                                                \
  SDE_EXPORT void* on_create(ScriptInstanceAllocator allocator)                                                        \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    void* instance = allocator(sizeof(ScriptDataT));                                                                   \
    new (instance) ScriptDataT{};                                                                                      \
    return instance;                                                                                                   \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_DESTROY(ScriptDataT)                                                               \
  SDE_EXPORT void on_destroy(ScriptInstanceDeallocator deallocator, void* self)                                        \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    reinterpret_cast<ScriptDataT*>(self)->~ScriptDataT();                                                              \
    deallocator(self);                                                                                                 \
  }

#ifndef SDE_SCRIPT_NAME
#define SDE_NATIVE_SCRIPT__REGISTER_NAME(ScriptDataT)                                                                  \
  SDE_EXPORT const char* on_get_name() { return __FILE__; }
#else
#define SDE_NATIVE_SCRIPT__REGISTER_NAME(ScriptDataT)                                                                  \
  SDE_EXPORT const char* on_get_name() { return SDE_SCRIPT_NAME; }
#endif  // SDE_SCRIPT_NAME


#ifndef SDE_SCRIPT_DESCRIPTION
#define SDE_NATIVE_SCRIPT__REGISTER_DESCRIPTION(ScriptDataT)                                                           \
  SDE_EXPORT const char* on_get_description() { return __FILE__; }
#else
#define SDE_NATIVE_SCRIPT__REGISTER_DESCRIPTION(ScriptDataT)                                                           \
  SDE_EXPORT const char* on_get_description() { return SDE_SCRIPT_DESCRIPTION; }
#endif  // SDE_SCRIPT_DESCRIPTION


#define SDE_NATIVE_SCRIPT__REGISTER_VERSION(ScriptDataT, fn)                                                           \
  SDE_EXPORT script_version_t on_version()                                                                             \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    ::sde::game::VArchive varchive;                                                                                    \
    ScriptDataT data;                                                                                                  \
    fn(&data, varchive);                                                                                               \
    return varchive.digest().value;                                                                                    \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_LOAD(ScriptDataT, fn)                                                              \
  SDE_EXPORT bool on_load(void* self, void* iarchive)                                                                  \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    auto& iar = *reinterpret_cast<::sde::game::IArchive*>(iarchive);                                                   \
    return fn(reinterpret_cast<ScriptDataT*>(self), iar);                                                              \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_SAVE(ScriptDataT, fn)                                                              \
  SDE_EXPORT bool on_save(void* self, void* oarchive)                                                                  \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    auto& oar = *reinterpret_cast<::sde::game::OArchive*>(oarchive);                                                   \
    return fn(reinterpret_cast<ScriptDataT*>(self), oar);                                                              \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_INITIALIZE(ScriptDataT, f)                                                         \
  SDE_EXPORT bool on_initialize(void* self, void* resources, const void* app_properties)                               \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    return f(                                                                                                          \
      reinterpret_cast<ScriptDataT*>(self),                                                                            \
      *reinterpret_cast<::sde::game::GameResources*>(resources),                                                       \
      *reinterpret_cast<const ::sde::AppProperties*>(app_properties));                                                 \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_UPDATE(ScriptDataT, f)                                                             \
  SDE_EXPORT bool on_update(void* self, void* resources, const void* app_properties)                                   \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    return f(                                                                                                          \
      reinterpret_cast<ScriptDataT*>(self),                                                                            \
      *reinterpret_cast<::sde::game::GameResources*>(resources),                                                       \
      *reinterpret_cast<const ::sde::AppProperties*>(app_properties));                                                 \
  }

#define SDE_NATIVE_SCRIPT__REGISTER(ScriptDataT, serialize, initialize, update)                                        \
  SDE_NATIVE_SCRIPT__REGISTER_CREATE(ScriptDataT);                                                                     \
  SDE_NATIVE_SCRIPT__REGISTER_DESTROY(ScriptDataT);                                                                    \
  SDE_NATIVE_SCRIPT__REGISTER_NAME(ScriptDataT);                                                                       \
  SDE_NATIVE_SCRIPT__REGISTER_DESCRIPTION(ScriptDataT);                                                                \
  SDE_NATIVE_SCRIPT__REGISTER_VERSION(ScriptDataT, serialize);                                                         \
  SDE_NATIVE_SCRIPT__REGISTER_LOAD(ScriptDataT, serialize);                                                            \
  SDE_NATIVE_SCRIPT__REGISTER_SAVE(ScriptDataT, serialize);                                                            \
  SDE_NATIVE_SCRIPT__REGISTER_INITIALIZE(ScriptDataT, initialize);                                                     \
  SDE_NATIVE_SCRIPT__REGISTER_UPDATE(ScriptDataT, update);


#define SDE_NATIVE_SCRIPT__REGISTER_AUTO(ScriptDataT)                                                                  \
  SDE_NATIVE_SCRIPT__REGISTER(ScriptDataT, serialize, initialize, update)
