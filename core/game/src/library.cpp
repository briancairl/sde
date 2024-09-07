// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/library.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

expected<void, LibraryError> LibraryCache::reload(LibraryData& library)
{
  auto lib_or_error = dl::Library::load(library.path.string().c_str());
  if (!lib_or_error.has_value())
  {
    SDE_LOG_ERROR_FMT("failed to open library: %s", lib_or_error.error().details);
    return make_unexpected(LibraryError::kLibraryMissing);
  }
  library.lib = std::move(lib_or_error).value();
  return {};
}

expected<void, LibraryError> LibraryCache::unload(LibraryData& library)
{
  library.lib.reset();
  return {};
}

expected<LibraryData, LibraryError> LibraryCache::generate(const asset::path& path, const LibraryFlags& flags)
{
  const auto absolute_path = asset::absolute(path);
  if (asset_path_lookup_.count(absolute_path) > 0)
  {
    SDE_LOG_ERROR("LibraryError::kLibraryAlreadyLoaded");
    return make_unexpected(LibraryError::kLibraryAlreadyLoaded);
  }

  LibraryData data{.flags = flags, .path = absolute_path, .lib = {}};

  auto ok_or_error = reload(data);
  if (!ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

void LibraryCache::when_created(LibraryHandle handle, const LibraryData* data)
{
  SDE_LOG_INFO_FMT("new library added: %s", data->path.string().c_str());
  asset_path_lookup_.emplace(
    std::piecewise_construct, std::forward_as_tuple(data->path), std::forward_as_tuple(handle, data));
}

void LibraryCache::when_removed([[maybe_unused]] LibraryHandle handle, const LibraryData* data)
{
  asset_path_lookup_.erase(data->path);
}

std::pair<LibraryHandle, const LibraryData*> LibraryCache::get_if(const asset::path& path) const
{
  const auto itr = asset_path_lookup_.find(asset::absolute(path));
  if (itr == std::end(asset_path_lookup_))
  {
    return {LibraryHandle::null(), nullptr};
  }
  return itr->second;
}

}  // namespace sde::game