// SDE
#include "sde/graphics/assets_io.hpp"
#include "sde/graphics/assets.hpp"
#include "sde/graphics/font_io.hpp"
#include "sde/graphics/image_io.hpp"
#include "sde/graphics/render_target_io.hpp"
#include "sde/graphics/shader_io.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/graphics/type_set_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::Assets>::operator()(binary_ofarchive& ar, const graphics::Assets& assets) const
{
  ar << named{"images", assets.images};
  ar << named{"fonts", assets.fonts};
  ar << named{"shaders", assets.shaders};
  ar << named{"textures", assets.textures};
  ar << named{"tile_sets", assets.tile_sets};
  ar << named{"type_sets", assets.type_sets};
  ar << named{"render_targets", assets.render_targets};
}


template <>
void load<binary_ifarchive, graphics::Assets>::operator()(binary_ifarchive& ar, graphics::Assets& assets) const
{
  ar >> named{"images", assets.images};
  ar >> named{"fonts", assets.fonts};
  ar >> named{"shaders", assets.shaders};
  ar >> named{"textures", assets.textures};
  ar >> named{"tile_sets", assets.tile_sets};
  ar >> named{"type_sets", assets.type_sets};
  ar >> named{"render_targets", assets.render_targets};
}

}  // namespace sde::serial
