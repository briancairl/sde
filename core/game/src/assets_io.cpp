// SDE
#include "sde/game/assets_io.hpp"
#include "sde/game/assets.hpp"
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive> void load<Archive, game::Assets>::operator()(Archive& ar, game::Assets& assets) const
{
  ar >> named{"graphics", _R(assets.graphics)};
  ar >> named{"audio", _R(assets.audio)};
}

template <typename Archive> void save<Archive, game::Assets>::operator()(Archive& ar, const game::Assets& assets) const
{
  ar << named{"graphics", _R(assets.graphics)};
  ar << named{"audio", _R(assets.audio)};
}

template struct load<binary_ifarchive, game::Assets>;
template struct save<binary_ofarchive, game::Assets>;

}  // namespace sde::serial
