/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_target.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/expected.hpp"
#include "sde/graphics/render_target_fwd.hpp"
#include "sde/graphics/render_target_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::graphics
{

struct NativeFrameBufferDeleter
{
  void operator()(native_frame_buffer_id_t id) const;
};

using NativeFrameBufferID = UniqueResource<native_frame_buffer_id_t, NativeFrameBufferDeleter>;

struct RenderTarget : Resource<RenderTarget>
{
  TextureHandle color_attachment = TextureHandle::null();
  NativeFrameBufferID native_id = NativeFrameBufferID{0};

  auto field_list()
  {
    return FieldList((Field{"color_attachment", color_attachment}), (_Stub{"native_id", native_id}));
  }
};

enum class RenderTargetError
{
  kElementAlreadyExists,
  kInvalidHandle,
  kInvalidColorAttachment,
};

std::ostream& operator<<(std::ostream& os, RenderTargetError error);

class RenderTargetCache : public ResourceCache<RenderTargetCache>
{
  friend fundemental_type;

private:
  expected<void, RenderTargetError> reload(dependencies deps, RenderTarget& render_target);
  expected<void, RenderTargetError> unload(dependencies deps, RenderTarget& render_target);
  expected<RenderTarget, RenderTargetError>
  generate(dependencies deps, TextureHandle color_attachment = TextureHandle::null());
};

}  // namespace sde::graphics
