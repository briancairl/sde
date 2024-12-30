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
#include "sde/game/library.hpp"
#include "sde/game/library_fwd.hpp"
#include "sde/game/library_handle.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/native_script_methods.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"

namespace sde::game
{

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
  /// Alias given to this script
  sde::string name = {};

  /// Source library for this script
  LibraryHandle library = LibraryHandle::null();

  /// Script methods loaded from library
  NativeScriptMethods methods = {};

  auto field_list() { return FieldList(Field{"name", name}, Field{"library", library}, Field{"methods", methods}); }
};

class NativeScriptCache : public ResourceCache<NativeScriptCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  NativeScriptHandle to_handle(const LibraryHandle& library) const;
  NativeScriptHandle to_handle(const sde::string& name) const;

private:
  sde::unordered_map<sde::string, NativeScriptHandle> name_to_native_script_lookup_;
  sde::unordered_map<LibraryHandle, NativeScriptHandle, ResourceHandleStdHash> library_to_native_script_lookup_;
  expected<void, NativeScriptError> reload(dependencies deps, NativeScriptData& script);
  expected<void, NativeScriptError> unload(dependencies deps, NativeScriptData& script);
  expected<NativeScriptData, NativeScriptError>
  generate(dependencies deps, const sde::string& name, const asset::path& path, const LibraryFlags& flags = {});
  expected<NativeScriptData, NativeScriptError>
  generate(dependencies deps, const sde::string& name, LibraryHandle library);
  expected<NativeScriptData, NativeScriptError> generate(dependencies deps, LibraryHandle library);
  void when_created(dependencies deps, NativeScriptHandle handle, NativeScriptData* data);
  void when_removed(dependencies deps, NativeScriptHandle handle, const NativeScriptData* data);
};

}  // namespace sde::game
