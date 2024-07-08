/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script_impl.hpp
 */
#pragma once

// EnTT
#include <entt/entt.hpp>

// Common
#include "sde/expected.hpp"
#include "sde/resource_cache.hpp"
#include "sde/serialization_binary_file.hpp"

// Audio
#include "sde/audio/sound_data_io.hpp"
#include "sde/audio/sound_io.hpp"

// Graphics
#include "sde/graphics/assets_io.hpp"
#include "sde/graphics/font_io.hpp"
#include "sde/graphics/image_io.hpp"
#include "sde/graphics/render_target_io.hpp"
#include "sde/graphics/shader_io.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/graphics/tile_set_io.hpp"
#include "sde/graphics/type_set_io.hpp"

// Game
#include "sde/game/assets.hpp"
#include "sde/game/script_runtime.hpp"
#include "sde/game/systems.hpp"
