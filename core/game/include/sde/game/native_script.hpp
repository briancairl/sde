/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>

// SDE
#include "sde/asset.hpp"
#include "sde/game/game_resources_fwd.hpp"
#include "sde/game/library.hpp"
#include "sde/game/library_fwd.hpp"
#include "sde/game/library_handle.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/native_script_runtime_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"

namespace sde::game
{

enum class NativeScriptCallError
{
  kNotInitialized,
  kNotUpdated
};

std::ostream& operator<<(std::ostream& os, NativeScriptCallError error);

struct NativeScriptFn : public Resource<NativeScriptFn>
{
  dl::Function<void*(ScriptInstanceAllocator)> on_create;
  dl::Function<void(ScriptInstanceDeallocator, void*)> on_destroy;
  dl::Function<const char*()> on_get_name;
  dl::Function<const char*()> on_get_description;
  dl::Function<script_version_t()> on_version;
  dl::Function<bool(void*, void*)> on_load;
  dl::Function<bool(void*, void*)> on_save;
  dl::Function<bool(void*, void*, const void*)> on_initialize;
  dl::Function<bool(void*, void*, const void*)> on_update;

  bool isValid() const;

  void reset();

  auto field_list()
  {
    // clang-format off
    return FieldList(
      _Stub{"on_create", on_create},
      _Stub{"on_destroy", on_destroy},
      _Stub{"on_get_name", on_get_name},
      _Stub{"on_get_description", on_get_description},
      _Stub{"on_version", on_version},
      _Stub{"on_load", on_load},
      _Stub{"on_save", on_save},
      _Stub{"on_initialize", on_initialize},
      _Stub{"on_update", on_update});
    // clang-format on
  }
};


template <typename ScriptT> class NativeScriptBase : public Resource<NativeScriptBase<ScriptT>>
{
  friend typename Resource<NativeScriptBase<ScriptT>>::fundemental_type;

public:
  NativeScriptBase() = default;

  void swap(NativeScriptBase<ScriptT>& other);

  bool reset(const dl::Library& library);

  constexpr bool isValid() const { return fn_.isValid(); }

  constexpr operator bool() const { return this->isValid(); }

  constexpr std::string_view name() const { return isValid() ? fn_.on_get_name() : "<INVALID>"; }

  constexpr script_version_t version() const { return isValid() ? fn_.on_version() : 0UL; }

protected:
  explicit NativeScriptBase(NativeScriptFn fn);
  ~NativeScriptBase() = default;
  NativeScriptBase(const NativeScriptBase<ScriptT>&) = delete;
  NativeScriptBase& operator=(const NativeScriptBase<ScriptT>&) = delete;

  NativeScriptFn fn_;

  auto field_list() { return FieldList(_Stub{"fn", fn_}); }
};

template <typename ScriptT> std::ostream& operator<<(std::ostream& os, const NativeScriptBase<ScriptT>& script);

class NativeScriptInstance : public NativeScriptBase<NativeScriptInstance>
{
  using Base = NativeScriptBase<NativeScriptInstance>;

public:
  NativeScriptInstance() = default;
  NativeScriptInstance(NativeScriptFn fn);
  ~NativeScriptInstance();
  NativeScriptInstance(NativeScriptInstance&& other);
  NativeScriptInstance& operator=(NativeScriptInstance&& other);

  void swap(NativeScriptInstance& other);

  void reset();

  constexpr bool isValid() const { return Base::isValid() and (instance_ != nullptr); }

  constexpr operator bool() const { return this->isValid(); }

  bool load(IArchive& ar) const;

  bool save(OArchive& ar) const;

  expected<void, NativeScriptCallError> initialize(GameResources& resources, const AppProperties& app_properties) const;
  expected<void, NativeScriptCallError> call(GameResources& resources, const AppProperties& app_properties) const;

private:
  friend NativeScript;
  using Base::isValid;
  using Base::reset;
  using Base::operator bool;

  NativeScriptInstance(const NativeScriptInstance&) = delete;
  NativeScriptInstance& operator=(const NativeScriptInstance&) = delete;

  mutable bool initialized_ = false;
  mutable void* instance_ = nullptr;
};

class NativeScript : public NativeScriptBase<NativeScript>
{
public:
  ~NativeScript() = default;
  NativeScript() = default;

  NativeScript(NativeScript&& other);
  NativeScript& operator=(NativeScript&& other);

  constexpr operator bool() const { return this->isValid(); }

  NativeScriptInstance instance() const;

private:
  NativeScript(const NativeScript&) = delete;
  NativeScript& operator=(const NativeScript&) = delete;
};

enum class NativeScriptError
{
  kInvalidHandle,
  kElementAlreadyExists,
  kScriptLibraryInvalid,
  kScriptLibraryMissingFunction,
};

std::ostream& operator<<(std::ostream& os, NativeScriptError error);

struct NativeScriptData : Resource<NativeScriptData>
{
  /// Source library for this script
  LibraryHandle library;
  /// Name of script
  sde::string name;
  /// Script itself
  NativeScript script;

  auto field_list() { return FieldList(Field{"library", library}, Field{"name", name}, _Stub{"script", script}); }
};

class NativeScriptCache : public ResourceCache<NativeScriptCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  NativeScriptHandle to_handle(const LibraryHandle& library) const;

private:
  sde::unordered_map<LibraryHandle, NativeScriptHandle, ResourceHandleStdHash> library_to_native_script_lookup_;
  expected<void, NativeScriptError> reload(dependencies deps, NativeScriptData& library);
  expected<void, NativeScriptError> unload(dependencies deps, NativeScriptData& library);
  expected<NativeScriptData, NativeScriptError>
  generate(dependencies deps, const asset::path& path, const LibraryFlags& flags = {});
  expected<NativeScriptData, NativeScriptError> generate(dependencies deps, LibraryHandle library);
  void when_created(dependencies deps, NativeScriptHandle handle, const NativeScriptData* data);
  void when_removed(dependencies deps, NativeScriptHandle handle, const NativeScriptData* data);
};

}  // namespace sde::game

namespace sde
{
template <> struct Hasher<game::NativeScript> : ResourceHasher
{};
template <> struct Hasher<game::NativeScriptInstance> : ResourceHasher
{};
}  // namespace sde