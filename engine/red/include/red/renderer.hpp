#pragma once

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/script.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/type_set_handle.hpp"

// RED
#include "red/components.hpp"

using namespace sde;
using namespace sde::game;

class Renderer : public Script<Renderer>
{
  friend script;

private:
  bool onInitialize(entt::registry& registry, Systems& systems, Assets& assets, const AppProperties& app);

  expected<void, ScriptError>
  onUpdate(entt::registry& registry, Systems& systems, const Assets& assets, const AppProperties& app);

  float scaling_ = 0.0F;
  graphics::RenderBuffer render_buffer_;
  graphics::ShaderHandle sprite_shader_;
  graphics::TypeSetHandle player_text_type_set_;
  graphics::ShaderHandle player_text_shader_;
};
