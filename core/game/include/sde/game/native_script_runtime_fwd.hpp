/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_runtime_fwd.hpp
 */
#pragma once

// SDE
#include "sde/app_fwd.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/assets_fwd.hpp"

using ScriptInstanceAllocator = void*(std::size_t);
using ScriptInstanceDeallocator = void(void*);
