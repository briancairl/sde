/**
 * @copyright 2024-present Brian Cairl
 *
 * @file typedef.hpp
 */
#pragma once

namespace sde::graphics
{

/// Indexing type
using index_t = unsigned long;

/// ID type used for enumerations
using enum_t = unsigned;

/// ID type used for shaders, ideally identical to the graphics API ID
using native_shader_id_t = unsigned;

/// ID type used for textures, ideally identical to the graphics API ID
using native_texture_id_t = unsigned;

/// ID type used for frame buffer targets, ideally identical to the graphics API ID
using native_frame_buffer_id_t = unsigned;

/// ID type used for vertex_buffers, ideally identical to the graphics API ID
using native_vertex_buffer_id_t = unsigned;

}  // namespace sde::graphics
