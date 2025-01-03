/**
 * @copyright 2024-present Brian Cairl
 *
 * @file library.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/asset.hpp"
#include "sde/dl/library.hpp"
#include "sde/game/library_fwd.hpp"
#include "sde/game/library_handle.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/unordered_map.hpp"

namespace sde::game
{

enum class LibraryError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kLibraryMissing,
  kLibraryAlreadyLoaded,
};

std::ostream& operator<<(std::ostream& os, LibraryError error);

struct LibraryFlags : Resource<LibraryFlags>
{
  bool required = false;

  auto field_list() { return FieldList(Field{"required", required}); }
};

struct LibraryData : Resource<LibraryData>
{
  LibraryFlags flags;
  asset::path path;
  dl::Library lib;

  auto field_list() { return FieldList(Field{"flags", flags}, Field{"path", path}, _Stub{"lib", lib}); }
};

class LibraryCache : public ResourceCache<LibraryCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  LibraryHandle to_handle(const asset::path& path) const;

private:
  sde::unordered_map<asset::path, LibraryHandle> asset_path_lookup_;
  expected<void, LibraryError> reload(dependencies deps, LibraryData& library);
  expected<void, LibraryError> unload(dependencies deps, LibraryData& library);
  expected<LibraryData, LibraryError>
  generate(dependencies deps, const asset::path& path, const LibraryFlags& flags = {});
  void when_created(dependencies deps, LibraryHandle handle, const LibraryData* data);
  void when_removed(dependencies deps, LibraryHandle handle, const LibraryData* data);
};

}  // namespace sde::game

namespace sde
{
template <> struct Hasher<game::LibraryFlags> : ResourceHasher
{};
}  // namespace sde