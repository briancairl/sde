/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script_runtime_fwd.hpp
 */
#pragma once

// C++ Standard Library
#include <memory>

namespace sde::game
{

class ScriptRuntime;
using ScriptRuntimeUPtr = std::unique_ptr<ScriptRuntime>;

}  // namespace sde::game
