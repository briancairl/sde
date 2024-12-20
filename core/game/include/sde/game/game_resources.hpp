/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assets.hpp
 */
#pragma once

// SDE
#include "sde/asset.hpp"
#include "sde/audio/sound.hpp"
#include "sde/audio/sound_data.hpp"
#include "sde/expected.hpp"
#include "sde/game/component.hpp"
#include "sde/game/entity.hpp"
#include "sde/game/library.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/registry.hpp"
#include "sde/game/scene.hpp"
#include "sde/graphics/font.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/memory.hpp"
#include "sde/resource_collection.hpp"

namespace sde::game
{

// clang-format off
class GameResources : public ResourceCollection
<
  ResourceCollectionEntry<"sound_data"_rl, audio::SoundDataCache>,
  ResourceCollectionEntry<"sounds"_rl, audio::SoundCache>,
  ResourceCollectionEntry<"images"_rl, graphics::ImageCache>,
  ResourceCollectionEntry<"fonts"_rl, graphics::FontCache>,
  ResourceCollectionEntry<"shaders"_rl, graphics::ShaderCache>,
  ResourceCollectionEntry<"textures"_rl, graphics::TextureCache>,
  ResourceCollectionEntry<"tile_sets"_rl, graphics::TileSetCache>,
  ResourceCollectionEntry<"type_sets"_rl, graphics::TypeSetCache>,
  ResourceCollectionEntry<"render_targets"_rl, graphics::RenderTargetCache>,
  ResourceCollectionEntry<"registry"_rl, Registry, false>,
  ResourceCollectionEntry<"entities"_rl, EntityCache>,
  ResourceCollectionEntry<"libraries"_rl, LibraryCache>,
  ResourceCollectionEntry<"scripts"_rl, NativeScriptCache>,
  ResourceCollectionEntry<"components"_rl, ComponentCache>,
  ResourceCollectionEntry<"scenes"_rl, SceneCache>
>
// clang-format on
{
public:
  explicit GameResources(asset::path root) : root_{std::move(root)} {}

  const asset::path& root() const { return root_; }

private:
  asset::path root_;
};

}  // namespace sde::game
