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

expected<LibraryData, LibraryError> LibraryCache::generate(const asset::path& path)
{
  LibraryData data{.path = path, .lib = {}};

  auto ok_or_error = reload(data);
  if (!ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

}  // namespace sde::game