// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::TileSetCache>::operator()(Archive& ar, const graphics::TileSetCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"tile_atlas", info.tile_atlas};
    ar << named{"tile_bounds", info.tile_bounds};
  }
}

template <typename Archive>
void load<Archive, graphics::TileSetCache>::operator()(Archive& ar, graphics::TileSetCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::TileSetHandle handle;
    ar >> named{"handle", handle};
    graphics::TextureHandle tile_atlas;
    ar >> named{"tile_atlas", tile_atlas};
    std::vector<Bounds2f> tile_bounds;
    ar >> named{"tile_bounds", tile_bounds};
    cache.insert(handle, tile_atlas, std::move(tile_bounds));
  }
}

template struct save<binary_ofarchive, graphics::TileSetCache>;
template struct load<binary_ifarchive, graphics::TileSetCache>;

}  // namespace sde::serial
