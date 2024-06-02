/**
 * @copyright 2024-present Brian Cairl
 *
 * @file debug_null.cpp
 */

// Tyl
#include "sde/graphics/debug.hpp"

#warning "Compiling with graphical device (OpenGL) debugging disabled"

namespace sde::graphics
{

void enable_native_debug_logs() {}

void disable_native_debug_logs() {}

void enable_native_error_logs() {}

void disable_native_error_logs() {}

}  // namespace sde::graphics
