/**
 * @copyright 2022-present Brian Cairl
 *
 * @file typedef.hpp
 */
#pragma once

namespace sde::audio
{

/// Handle type used for context backed
using context_handle_t = void*;

/// Handle type used for sound playback devices
using device_handle_t = void*;

/// Handle type used for sound sinks (a.k.a listeners)
using listener_handle_t = void*;

/// Handle type used for sound sources
using source_handle_t = unsigned;

/// Handle type used for sound buffers
using buffer_handle_t = unsigned;

}  // namespace sde::audio
