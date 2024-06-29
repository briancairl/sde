// C++ Standard Library
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, RenderTargetError error)
{
  switch (error)
  {
  case RenderTargetError::kInvalidColorAttachment:
    return os << "InvalidColorAttachment";
  case RenderTargetError::kElementAlreadyExists:
    return os << "ElementAlreadyExists";
  }
  return os;
}

void NativeFrameBufferDeleter::operator()(native_frame_buffer_id_t id) const { glDeleteFramebuffers(1, &id); }

RenderTargetCache::RenderTargetCache() : cache_base{}
{
  cache_base::handle_to_value_cache_.try_emplace(
    RenderTargetHandle::null(),
    RenderTargetInfo{.color_attachment = TextureHandle::null(), .native_id = NativeFrameBufferID{0}});
}

expected<RenderTargetInfo, RenderTargetError>
RenderTargetCache::generate(TextureHandle color_attachment, const TextureCache& texture_cache)
{
  if (color_attachment.isNull())
  {
    return unexpected<RenderTargetError>{RenderTargetError::kInvalidColorAttachment};
  }

  const auto* texture_info = texture_cache.get_if(color_attachment);

  if (texture_info == nullptr)
  {
    return unexpected<RenderTargetError>{RenderTargetError::kInvalidColorAttachment};
  }

  GLuint texture_framebuffer;
  glGenFramebuffers(1, &texture_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, texture_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_info->native_id, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return RenderTargetInfo{.color_attachment = color_attachment, .native_id = NativeFrameBufferID{texture_framebuffer}};
}

}  // namespace sde::graphics
