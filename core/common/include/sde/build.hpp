/**
 * @copyright 2024-present Brian Cairl
 *
 * @file crtp.hpp
 */
#pragma once

#define SDE_VERSION_MINOR 0
#define SDE_VERSION_MAJOR 0

#if defined(SDE_DEBUG_ENABLED_BY_BUILD_SYSTEM) || !(defined(NDEBUG) && defined(_NDEBUG))
#define SDE_DEBUG_ENABLED 1
#warning "SDE building in debug mode"
#else
#define SDE_DEBUG_ENABLED 0
#endif