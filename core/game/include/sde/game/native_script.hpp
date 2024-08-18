/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script.hpp
 */
#pragma once

// SDE
#include "sde/asset.hpp"
#include "sde/dl/library.hpp"
#include "sde/game/library_fwd.hpp"
#include "sde/game/library_handle.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/native_script_runtime_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"

namespace sde::game
{

enum class NativeScriptError
{
  kInvalidHandle,
  kScriptLibraryInvalid,
  kScriptLibraryMissingFunction,
};

class NativeScript : public Resource<NativeScript>
{
  friend fundemental_type;

public:
  NativeScript() = default;
  ~NativeScript();

  NativeScript(NativeScript&& other);
  NativeScript& operator=(NativeScript&& other);

  void swap(NativeScript& other);

  void reset();
  bool reset(const dl::Library& library);

  constexpr operator bool() const { return instance_ != nullptr; }

  bool load(IArchive& ar) const;
  bool save(OArchive& ar) const;
  bool call(Assets& assets, AppState& app_state, const AppProperties& app_properties);

private:
  NativeScript(const NativeScript&) = delete;
  NativeScript& operator=(const NativeScript&) = delete;

  bool initialized_ = false;
  void* instance_ = nullptr;

  dl::Function<void*(void)> on_create_;
  dl::Function<void(void*)> on_destroy_;
  dl::Function<bool(void*)> on_load_;
  dl::Function<bool(void*)> on_save_;
  dl::Function<bool(void*, void*, const void*)> on_initialize_;
  dl::Function<bool(void*, void*, const void*)> on_update_;

  auto field_list()
  {
    // clang-format off
    return FieldList(
      _Stub{"on_create", on_create_},
      _Stub{"on_destroy", on_destroy_},
      _Stub{"on_load", on_load_},
      _Stub{"on_save", on_save_},
      _Stub{"on_initialize", on_initialize_},
      _Stub{"on_update", on_update_});
    // clang-format on
  }
};

struct NativeScriptData : Resource<NativeScriptData>
{
  LibraryHandle library;
  NativeScript instance;

  auto field_list() { return FieldList(Field{"library", library}, _Stub{"instance", instance}); }
};

}  // namespace sde::game

namespace sde
{

template <> struct ResourceCacheTypes<game::NativeScriptCache>
{
  using error_type = game::NativeScriptError;
  using handle_type = game::NativeScriptHandle;
  using value_type = game::NativeScriptData;
};

}  // namespace sde

namespace sde::game
{

class NativeScriptCache : public ResourceCache<NativeScriptCache>
{
  friend fundemental_type;

public:
  explicit NativeScriptCache(LibraryCache& libraries);

private:
  LibraryCache* libraries_;
  expected<void, NativeScriptError> reload(NativeScriptData& library);
  expected<void, NativeScriptError> unload(NativeScriptData& library);
  expected<NativeScriptData, NativeScriptError> generate(const asset::path& path);
  expected<NativeScriptData, NativeScriptError> generate(LibraryHandle library);
};

}  // namespace sde::game
