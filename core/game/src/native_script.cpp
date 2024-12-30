// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/library.hpp"
#include "sde/game/native_script.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, NativeScriptError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASES_FOR_RESOURCE_CACHE_ERRORS(NativeScriptError)
    SDE_OS_ENUM_CASE(NativeScriptError::kScriptLibraryInvalid)
    SDE_OS_ENUM_CASE(NativeScriptError::kScriptLibraryMissingFunction)
  }
  return os;
}

NativeScriptHandle NativeScriptCache::to_handle(const LibraryHandle& library) const
{
  const auto itr = library_to_native_script_lookup_.find(library);
  if (itr == std::end(library_to_native_script_lookup_))
  {
    return NativeScriptHandle::null();
  }
  return itr->second;
}


NativeScriptHandle NativeScriptCache::to_handle(const sde::string& name) const
{
  const auto itr = name_to_native_script_lookup_.find(name);
  if (itr == std::end(name_to_native_script_lookup_))
  {
    return NativeScriptHandle::null();
  }
  return itr->second;
}


void NativeScriptCache::when_created(
  [[maybe_unused]] dependencies deps,
  NativeScriptHandle handle,
  NativeScriptData* script)
{
  if (script->name.empty())
  {
    script->name = script->methods.on_get_type_name();
    SDE_LOG_ERROR() << "NativeScript alias not set. Using default alias: " << SDE_OSNV(script->name);
  }

  {
    const auto [itr, added] = name_to_native_script_lookup_.emplace(script->name, handle);
    SDE_ASSERT_TRUE(added) << itr->first << ": is not a unique script alias for " << SDE_OSNV(handle);
  }

  {
    const auto [itr, added] = library_to_native_script_lookup_.emplace(script->library, handle);
    SDE_ASSERT_TRUE(added) << itr->first << ": is not a unique script library for " << SDE_OSNV(handle);
  }
}

void NativeScriptCache::when_removed(
  [[maybe_unused]] dependencies deps,
  NativeScriptHandle handle,
  const NativeScriptData* script)
{
  name_to_native_script_lookup_.erase(script->name);
  library_to_native_script_lookup_.erase(script->library);
}

expected<void, NativeScriptError> NativeScriptCache::reload(dependencies deps, NativeScriptData& script)
{
  const auto* library_ptr = deps.get<LibraryCache>().get_if(script.library);

  if (library_ptr == nullptr)
  {
    SDE_LOG_ERROR() << "ScriptLibraryInvalid: " << SDE_OSNV(script.library);
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }

  if (!script.methods.reset(library_ptr->lib))
  {
    SDE_LOG_ERROR() << "ScriptLibraryMissingFunction: " << SDE_OSNV(library_ptr->lib);
    return make_unexpected(NativeScriptError::kScriptLibraryMissingFunction);
  }

  return {};
}

expected<void, NativeScriptError> NativeScriptCache::unload(dependencies deps, NativeScriptData& script) { return {}; }

expected<NativeScriptData, NativeScriptError> NativeScriptCache::generate(dependencies deps, LibraryHandle library)
{
  return this->generate(deps, {}, library);
}

expected<NativeScriptData, NativeScriptError>
NativeScriptCache::generate(dependencies deps, const sde::string& name, LibraryHandle library)
{
  NativeScriptData data{.name = name, .library = library};

  if (const auto ok_or_error = reload(deps, data); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

expected<NativeScriptData, NativeScriptError> NativeScriptCache::generate(
  dependencies deps,
  const sde::string& name,
  const asset::path& path,
  const LibraryFlags& flags)
{
  auto library_or_error = deps.get<LibraryCache>().find_or_create(path, deps, path, flags);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR() << "ScriptLibraryInvalid: " << library_or_error.error();
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }
  return this->generate(deps, name, library_or_error->handle);
}

}  // namespace sde::game