/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_runtime.hpp
 */
#pragma once

// SDE
#include "sde/app_properties.hpp"
#include "sde/dl/export.hpp"
#include "sde/expected.hpp"
#include "sde/game/archive.hpp"
#include "sde/game/assets.hpp"
#include "sde/game/entity.hpp"
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
#include "sde/serial/std/vector.hpp"
#include "sde/serialization.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"


#define SDE_NATIVE_SCRIPT__REGISTER_CREATE(InstanceDataT)                                                              \
  SDE_EXPORT void* on_create(ScriptInstanceAllocator allocator)                                                        \
  {                                                                                                                    \
    void* instance = allocator(sizeof(InstanceDataT));                                                                 \
    new (instance) InstanceDataT{};                                                                                    \
    return instance;                                                                                                   \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_DESTROY(InstanceDataT)                                                             \
  SDE_EXPORT void on_destroy(ScriptInstanceDeallocator deallocator, void* self)                                        \
  {                                                                                                                    \
    reinterpret_cast<InstanceDataT*>(self)->~InstanceDataT();                                                          \
    deallocator(self);                                                                                                 \
  }


#define SDE_NATIVE_SCRIPT__REGISTER_LOAD(InstanceDataT, f)                                                             \
  SDE_EXPORT bool on_load(void* self, void* iarchive)                                                                  \
  {                                                                                                                    \
    return f(reinterpret_cast<InstanceDataT*>(self), *reinterpret_cast<::sde::game::IArchive*>(iarchive));             \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_SAVE(InstanceDataT, f)                                                             \
  SDE_EXPORT bool on_save(void* self, void* oarchive)                                                                  \
  {                                                                                                                    \
    return f(reinterpret_cast<InstanceDataT*>(self), *reinterpret_cast<::sde::game::OArchive*>(oarchive));             \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_INITIALIZE(InstanceDataT, f)                                                       \
  SDE_EXPORT bool on_initialize(void* self, void* assets, const void* app_properties)                                  \
  {                                                                                                                    \
    return f(                                                                                                          \
      reinterpret_cast<InstanceDataT*>(self),                                                                          \
      *reinterpret_cast<::sde::game::Assets*>(assets),                                                                 \
      *reinterpret_cast<const ::sde::AppProperties*>(app_properties));                                                 \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_UPDATE(InstanceDataT, f)                                                           \
  SDE_EXPORT bool on_update(void* self, void* assets, const void* app_properties)                                      \
  {                                                                                                                    \
    return f(                                                                                                          \
      reinterpret_cast<InstanceDataT*>(self),                                                                          \
      *reinterpret_cast<::sde::game::Assets*>(assets),                                                                 \
      *reinterpret_cast<const ::sde::AppProperties*>(app_properties));                                                 \
  }

#define SDE_NATIVE_SCRIPT__REGISTER(InstanceDataT, load, save, initialize, update)                                     \
  SDE_NATIVE_SCRIPT__REGISTER_CREATE(InstanceDataT);                                                                   \
  SDE_NATIVE_SCRIPT__REGISTER_DESTROY(InstanceDataT);                                                                  \
  SDE_NATIVE_SCRIPT__REGISTER_LOAD(InstanceDataT, load);                                                               \
  SDE_NATIVE_SCRIPT__REGISTER_SAVE(InstanceDataT, save);                                                               \
  SDE_NATIVE_SCRIPT__REGISTER_INITIALIZE(InstanceDataT, initialize);                                                   \
  SDE_NATIVE_SCRIPT__REGISTER_UPDATE(InstanceDataT, update);


#define SDE_NATIVE_SCRIPT__REGISTER_AUTO(InstanceDataT)                                                                \
  SDE_NATIVE_SCRIPT__REGISTER(InstanceDataT, load, save, initialize, update)
