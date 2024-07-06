// SDE
#include "sde/game/assets_io.hpp"
#include "sde/audio/assets_io.hpp"
#include "sde/game/assets.hpp"
#include "sde/graphics/assets_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, game::Assets>::operator()(binary_ofarchive& ar, const game::Assets& assets) const
{
  ar << named{"graphics", assets.graphics};
  ar << named{"audio", assets.audio};
}


template <> void load<binary_ifarchive, game::Assets>::operator()(binary_ifarchive& ar, game::Assets& assets) const
{
  ar >> named{"graphics", assets.graphics};
  ar >> named{"audio", assets.audio};
}

}  // namespace sde::serial
