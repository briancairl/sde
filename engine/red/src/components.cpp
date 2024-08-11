// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/archive.hpp"
#include "sde/game/scene.hpp"
#include "sde/game/script_impl.hpp"
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/resource_io.hpp"

// RED
#include "red/components.hpp"

void addComponentsToScene(sde::game::Scene& scene)
{
  scene.addComponent<Info>();
  scene.addComponent<Size>();
  scene.addComponent<DebugWireFrame>();
  scene.addComponent<Position>();
  scene.addComponent<TransformQuery>();
  scene.addComponent<Dynamics>();
  scene.addComponent<Focused>();
  scene.addComponent<Background>();
  scene.addComponent<Midground>();
  scene.addComponent<Foreground>();
  scene.addComponent<sde::graphics::TileMap>();
  scene.addComponent<sde::graphics::AnimatedSprite>();
  scene.addComponent<sde::graphics::Sprite>();
}