// SDE
#include "sde/game/component_runtime.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/tile_map.hpp"

// RED
#include "red/components/common.hpp"


SDE_COMPONENT__REGISTER(Info, Info);
SDE_COMPONENT__REGISTER(Size, Size);
SDE_COMPONENT__REGISTER(DebugWireFrame, DebugWireFrame);
SDE_COMPONENT__REGISTER(Position, Position);
SDE_COMPONENT__REGISTER(TransformQuery, TransformQuery);
SDE_COMPONENT__REGISTER(Dynamics, Dynamics);
SDE_COMPONENT__REGISTER(Focused, Focused);
SDE_COMPONENT__REGISTER(Background, Background);
SDE_COMPONENT__REGISTER(Midground, Midground);
SDE_COMPONENT__REGISTER(Foreground, Foreground);
SDE_COMPONENT__REGISTER(TileMap, sde::graphics::TileMap);
SDE_COMPONENT__REGISTER(AnimatedSprite, sde::graphics::AnimatedSprite);
SDE_COMPONENT__REGISTER(Sprite, sde::graphics::Sprite);
