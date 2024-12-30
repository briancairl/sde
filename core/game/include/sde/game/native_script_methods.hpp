/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_methods.hpp
 */
#pragma once

// SDE
#include "sde/dl/library.hpp"
#include "sde/game/native_script_typedefs.hpp"
#include "sde/resource.hpp"

namespace sde::game
{

struct NativeScriptMethods : public Resource<NativeScriptMethods>
{
  dl::Function<void*(ScriptInstanceAllocator)> on_create;
  dl::Function<void(ScriptInstanceDeallocator, void*)> on_destroy;
  dl::Function<const char*()> on_get_type_name;
  dl::Function<const char*()> on_get_description;
  dl::Function<script_version_t()> on_get_version;
  dl::Function<bool(void*, void*)> on_load;
  dl::Function<bool(void*, void*)> on_save;
  dl::Function<bool(void*, void*, const void*)> on_initialize;
  dl::Function<bool(void*, void*, const void*)> on_update;
  dl::Function<bool(void*, void*, const void*)> on_shutdown;

  bool isValid() const;

  void reset();

  bool reset(const dl::Library& library);

  auto field_list()
  {
    // clang-format off
    return FieldList(
      _Stub{"on_create", on_create},
      _Stub{"on_destroy", on_destroy},
      _Stub{"on_get_type_name", on_get_type_name},
      _Stub{"on_get_description", on_get_description},
      _Stub{"on_get_version", on_get_version},
      _Stub{"on_load", on_load},
      _Stub{"on_save", on_save},
      _Stub{"on_initialize", on_initialize},
      _Stub{"on_update", on_update},
      _Stub{"on_shutdown", on_shutdown}
    );
    // clang-format on
  }
};

}  // namespace sde::game
