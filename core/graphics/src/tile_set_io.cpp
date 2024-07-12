// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::TileSetHandle>::operator()(Archive& ar, const graphics::TileSetHandle& handle) const
{
  ar << handle.fundemental();
}

template <typename Archive>
void load<Archive, graphics::TileSetHandle>::operator()(Archive& ar, graphics::TileSetHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename Archive>
void save<Archive, graphics::TileSetCache>::operator()(Archive& ar, const graphics::TileSetCache& cache) const
{
  ar << cache.fundemental();
}

template <typename Archive>
void load<Archive, graphics::TileSetCache>::operator()(Archive& ar, graphics::TileSetCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::TileSetHandle>;
template struct load<binary_ifarchive, graphics::TileSetHandle>;
template struct save<binary_ofarchive, graphics::TileSetCache>;
template struct load<binary_ifarchive, graphics::TileSetCache>;

}  // namespace sde::serial
