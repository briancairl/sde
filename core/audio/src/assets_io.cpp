// SDE
#include "sde/audio/assets_io.hpp"
#include "sde/audio/assets.hpp"
#include "sde/audio/sound_data_io.hpp"
#include "sde/audio/sound_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, audio::Assets>::operator()(binary_ofarchive& ar, const audio::Assets& assets) const
{
  ar << named{"sound_data", assets.sound_data};
  ar << named{"sounds", assets.sounds};
}


template <> void load<binary_ifarchive, audio::Assets>::operator()(binary_ifarchive& ar, audio::Assets& assets) const
{
  ar >> named{"sound_data", assets.sound_data};
  ar >> named{"sounds", assets.sounds};
}

}  // namespace sde::serial
