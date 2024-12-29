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
    SDE_OS_ENUM_CASE(NativeScriptError::kInvalidHandle)
    SDE_OS_ENUM_CASE(NativeScriptError::kElementAlreadyExists)
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


void NativeScriptCache::when_created(
  [[maybe_unused]] dependencies deps,
  NativeScriptHandle handle,
  const NativeScriptData* data)
{
  library_to_native_script_lookup_.emplace(data->library, handle);
}

void NativeScriptCache::when_removed(
  [[maybe_unused]] dependencies deps,
  NativeScriptHandle handle,
  const NativeScriptData* data)
{
  library_to_native_script_lookup_.erase(data->library);
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
  NativeScriptData data{.library = library};

  if (const auto ok_or_error = reload(deps, data); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

expected<NativeScriptData, NativeScriptError>
NativeScriptCache::generate(dependencies deps, const asset::path& path, const LibraryFlags& flags)
{
  auto library_or_error = deps.get<LibraryCache>().find_or_create(path, deps, path, flags);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR() << "ScriptLibraryInvalid: " << library_or_error.error();
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }
  return this->generate(deps, library_or_error->handle);
}

}  // namespace sde::game