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
#include "sde/format.hpp"
#include "sde/game/archive.hpp"
#include "sde/game/entity.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_header.hpp"
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
  constexpr bool initialized() const { return !sde::game::native_script_header::name.empty(); }
  constexpr std::string_view name() const { return sde::game::native_script_header::name; }
  constexpr script_id_t uid() const { return sde::game::native_script_header::uid; }
  constexpr script_version_t version() const { return sde::game::native_script_header::version; }
  const char* guid() const { return sde::format("%s-%lu", name().data(), uid()); }
};

namespace sde::detail
{

template <typename InspectFn> struct inspect_via_serialize
{
public:
  explicit inspect_via_serialize(
    const char* inspect_object_name,
    const char* inspect_method_name,
    InspectFn inspect_fn) :
      valid_{true},
      inspect_object_name_{inspect_object_name},
      inspect_method_name_{inspect_method_name},
      inspect_fn_{std::move(inspect_fn)}
  {}

  operator bool() const { return valid_; }

  template <typename T> void operator&(const T& v)
  {
    if constexpr (sde::is_field_v<T>)
    {
      this->operator&(v.get());
    }
    else if constexpr (std::is_array_v<T> or sde::is_iterable_v<T>)
    {
      for (auto& element : v)
      {
        this->operator&(element);
      }
    }
    else if constexpr (sde::is_resource_handle_v<T>)
    {
      if (v.isNull())
      {
        return;
      }
      else if (auto ok_or_error = inspect_fn_(v); ok_or_error.has_value())
      {
        SDE_LOG_DEBUG() << inspect_object_name_ << "->" << inspect_method_name_ << '(' << sde::type_name<T>() << '='
                        << v << ')';
      }
      else
      {
        SDE_LOG_ERROR() << inspect_object_name_ << "->" << inspect_method_name_ << '(' << sde::type_name<T>() << '='
                        << v << ") failed with error: " << ok_or_error.error();
        valid_ = false;
      }
    }
  }

private:
  bool valid_ = true;
  const char* inspect_object_name_;
  const char* inspect_method_name_;
  InspectFn inspect_fn_;
};

}  // namespace sde::detail


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

#ifndef SDE_SCRIPT_TYPE_NAME
#define SDE_NATIVE_SCRIPT__REGISTER_NAME(ScriptDataT)                                                                  \
  SDE_EXPORT const char* on_get_type_name() { return __FILE__; }
#else
#define SDE_NATIVE_SCRIPT__REGISTER_NAME(ScriptDataT)                                                                  \
  SDE_EXPORT const char* on_get_type_name() { return SDE_SCRIPT_TYPE_NAME; }
#endif  // SDE_SCRIPT_TYPE_NAME


#ifndef SDE_SCRIPT_DESCRIPTION
#define SDE_NATIVE_SCRIPT__REGISTER_DESCRIPTION(ScriptDataT)                                                           \
  SDE_EXPORT const char* on_get_description() { return __FILE__; }
#else
#define SDE_NATIVE_SCRIPT__REGISTER_DESCRIPTION(ScriptDataT)                                                           \
  SDE_EXPORT const char* on_get_description() { return SDE_SCRIPT_DESCRIPTION; }
#endif  // SDE_SCRIPT_DESCRIPTION


#ifndef SDE_SCRIPT_VERSION
#define SDE_NATIVE_SCRIPT__REGISTER_VERSION(ScriptDataT, fn)                                                           \
  SDE_EXPORT script_version_t on_get_version()                                                                         \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    ::sde::game::VArchive varchive;                                                                                    \
    ScriptDataT data;                                                                                                  \
    fn(&data, varchive);                                                                                               \
    return varchive.digest().value;                                                                                    \
  }
#else
#define SDE_NATIVE_SCRIPT__REGISTER_VERSION(ScriptDataT, fn)                                                           \
  SDE_EXPORT script_version_t on_get_version() { return SDE_SCRIPT_VERSION; }
#endif  // SDE_SCRIPT_VERSION

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

#define SDE_NATIVE_SCRIPT__REGISTER_INITIALIZE(ScriptDataT, f, inspect)                                                \
  SDE_EXPORT bool on_initialize(void* self, void* resources, const void* app_properties)                               \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    auto* _data = reinterpret_cast<ScriptDataT*>(self);                                                                \
    auto& _resources = *reinterpret_cast<::sde::game::GameResources*>(resources);                                      \
    if (f(_data, _resources, *reinterpret_cast<const ::sde::AppProperties*>(app_properties)))                          \
    {                                                                                                                  \
      sde::detail::inspect_via_serialize ar{                                                                           \
        __SDE_STR_EXPR(ScriptDataT), "borrowing", [r = _resources.all()](const auto& handle) mutable {                 \
          return r.borrow(handle);                                                                                     \
        }};                                                                                                            \
      return inspect(_data, ar) and ar;                                                                                \
    }                                                                                                                  \
    return false;                                                                                                      \
  }

#define SDE_NATIVE_SCRIPT__REGISTER_SHUTDOWN(ScriptDataT, f, inspect)                                                  \
  SDE_EXPORT bool on_shutdown(void* self, void* resources, const void* app_properties)                                 \
  {                                                                                                                    \
    static_assert(std::is_base_of_v<native_script_data, ScriptDataT>);                                                 \
    auto* _data = reinterpret_cast<ScriptDataT*>(self);                                                                \
    auto& _resources = *reinterpret_cast<::sde::game::GameResources*>(resources);                                      \
    if (f(_data, _resources, *reinterpret_cast<const ::sde::AppProperties*>(app_properties)))                          \
    {                                                                                                                  \
      sde::detail::inspect_via_serialize ar{                                                                           \
        __SDE_STR_EXPR(ScriptDataT), "restoring", [r = _resources.all()](const auto& handle) mutable {                 \
          return r.restore(handle);                                                                                    \
        }};                                                                                                            \
      return inspect(_data, ar) and ar;                                                                                \
    }                                                                                                                  \
    return false;                                                                                                      \
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

#define SDE_NATIVE_SCRIPT__REGISTER(ScriptDataT, serialize, initialize, update, shutdown)                              \
  SDE_NATIVE_SCRIPT__REGISTER_CREATE(ScriptDataT);                                                                     \
  SDE_NATIVE_SCRIPT__REGISTER_DESTROY(ScriptDataT);                                                                    \
  SDE_NATIVE_SCRIPT__REGISTER_NAME(ScriptDataT);                                                                       \
  SDE_NATIVE_SCRIPT__REGISTER_DESCRIPTION(ScriptDataT);                                                                \
  SDE_NATIVE_SCRIPT__REGISTER_VERSION(ScriptDataT, serialize);                                                         \
  SDE_NATIVE_SCRIPT__REGISTER_LOAD(ScriptDataT, serialize);                                                            \
  SDE_NATIVE_SCRIPT__REGISTER_SAVE(ScriptDataT, serialize);                                                            \
  SDE_NATIVE_SCRIPT__REGISTER_INITIALIZE(ScriptDataT, initialize, serialize);                                          \
  SDE_NATIVE_SCRIPT__REGISTER_UPDATE(ScriptDataT, update);                                                             \
  SDE_NATIVE_SCRIPT__REGISTER_SHUTDOWN(ScriptDataT, shutdown, serialize);


#define SDE_NATIVE_SCRIPT__REGISTER_AUTO(ScriptDataT)                                                                  \
  SDE_NATIVE_SCRIPT__REGISTER(ScriptDataT, serialize, initialize, update, shutdown)
