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
#include "sde/geometry_io.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/optional.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"

// Game
#include "sde/game/assets.hpp"
#include "sde/game/script_runtime.hpp"
#include "sde/game/systems.hpp"
