// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/library.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, LibraryError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(LibraryError::kInvalidHandle)
    SDE_OS_ENUM_CASE(LibraryError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(LibraryError::kLibraryMissing)
    SDE_OS_ENUM_CASE(LibraryError::kLibraryAlreadyLoaded)
  }
  return os;
}

expected<void, LibraryError> LibraryCache::reload([[maybe_unused]] dependencies deps, LibraryData& library)
{
  auto lib_or_error = dl::Library::load(library.path.string().c_str());
  if (!lib_or_error.has_value())
  {
    SDE_LOG_ERROR() << "Failed to open library: " << lib_or_error.error();
    return make_unexpected(LibraryError::kLibraryMissing);
  }
  SDE_LOG_INFO() << "Library loaded: " << SDE_OSNV(library.path);
  library.lib = std::move(lib_or_error).value();
  return {};
}

expected<void, LibraryError> LibraryCache::unload([[maybe_unused]] dependencies deps, LibraryData& library)
{
  library.lib.reset();
  return {};
}

expected<LibraryData, LibraryError>
LibraryCache::generate(dependencies deps, const asset::path& path, const LibraryFlags& flags)
{
  const auto absolute_path = asset::absolute(path);
  if (asset_path_lookup_.count(absolute_path) > 0)
  {
    SDE_LOG_ERROR() << "LibraryAlreadyLoaded: " << SDE_OSNV(absolute_path);
    return make_unexpected(LibraryError::kLibraryAlreadyLoaded);
  }

  LibraryData data{.flags = flags, .path = path, .lib = {}};

  auto ok_or_error = reload(deps, data);
  if (!ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

void LibraryCache::when_created([[maybe_unused]] dependencies deps, LibraryHandle handle, const LibraryData* data)
{
  SDE_LOG_INFO() << "New library added: " << SDE_OSNV(handle) << " : " << SDE_OSNV(data->path);
  asset_path_lookup_.emplace(asset::absolute(data->path), handle);
}

void LibraryCache::when_removed(
  [[maybe_unused]] dependencies deps,
  [[maybe_unused]] LibraryHandle handle,
  const LibraryData* data)
{
  asset_path_lookup_.erase(asset::absolute(data->path));
}

LibraryHandle LibraryCache::to_handle(const asset::path& path) const
{
  const auto itr = asset_path_lookup_.find(asset::absolute(path));
  if (itr == std::end(asset_path_lookup_))
  {
    return LibraryHandle::null();
  }
  return itr->second;
}

}  // namespace sde::game