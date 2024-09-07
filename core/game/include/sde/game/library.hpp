/**
 * @copyright 2024-present Brian Cairl
 *
 * @file library.hpp
 */
#pragma once

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
  kInvalidHandle,
  kLibraryMissing,
  kLibraryAlreadyLoaded,
};

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

}  // namespace sde::game

namespace sde
{

template <> struct Hasher<game::LibraryFlags> : ResourceHasher
{};

template <> struct ResourceCacheTypes<game::LibraryCache>
{
  using error_type = game::LibraryError;
  using handle_type = game::LibraryHandle;
  using value_type = game::LibraryData;
};

}  // namespace sde

namespace sde::game
{

class LibraryCache : public ResourceCache<LibraryCache>
{
  friend fundemental_type;

public:
  using fundemental_type::get_if;

  std::pair<LibraryHandle, const LibraryData*> get_if(const asset::path& path) const;

private:
  sde::unordered_map<asset::path, std::pair<LibraryHandle, const LibraryData*>> asset_path_lookup_;

  expected<void, LibraryError> reload(LibraryData& library);
  expected<void, LibraryError> unload(LibraryData& library);
  expected<LibraryData, LibraryError> generate(const asset::path& path, const LibraryFlags& flags = {});
  void when_created(LibraryHandle handle, const LibraryData* data);
  void when_removed(LibraryHandle handle, const LibraryData* data);
};

}  // namespace sde::game
