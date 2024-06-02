/**
 * @copyright 2021-present Brian Cairl
 *
 * @file debug.hpp
 */
#pragma once

namespace sde::graphics
{

/**
 * @brief Enables global graphics all debug logs
 */
void enable_native_debug_logs();

/**
 * @brief Disables global graphics all debug logs
 */
void disable_native_debug_logs();

/**
 * @brief Enables global graphics error logs, only
 */
void enable_native_error_logs();

/**
 * @brief Disables global graphics error logs, only
 */
void disable_native_error_logs();

}  // namespace sde::graphics
