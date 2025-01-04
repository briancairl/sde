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
    SDE_OS_ENUM_CASES_FOR_RESOURCE_CACHE_ERRORS(RenderTargetError)
    SDE_OS_ENUM_CASE(RenderTargetError::kInvalidColorAttachment)
  }
  return os;
}

void NativeFrameBufferDeleter::operator()(native_frame_buffer_id_t id) const { glDeleteFramebuffers(1, &id); }

void RenderTarget::reset(const Vec4f& color) const
{
  glBindFramebuffer(GL_FRAMEBUFFER, this->native_id);
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
}

expected<void, RenderTargetError> RenderTargetCache::reset(RenderTargetHandle render_target, const Vec4f& color)
{
  if (auto itr = handle_to_value_cache_.find(render_target); itr == std::end(handle_to_value_cache_))
  {
    return make_unexpected(RenderTargetError::kInvalidHandle);
  }
  else
  {
    RenderTargetCache::reset(itr->second.get(), color);
  }
  return {};
}

void RenderTargetCache::reset(const RenderTarget& render_target, const Vec4f& color) { render_target.reset(color); }

expected<void, RenderTargetError> RenderTargetCache::reload(dependencies deps, RenderTarget& render_target)
{
  if (render_target.color_attachment.isNull())
  {
    SDE_LOG_DEBUG() << "Default Frame Buffer";
    render_target.native_id = NativeFrameBufferID{0};
    return {};
  }
  auto color_attachment = deps(render_target.color_attachment);
  if (!color_attachment)
  {
    SDE_LOG_ERROR() << "InvalidColorAttachment: " << SDE_OSNV(render_target.color_attachment);
    return make_unexpected(RenderTargetError::kInvalidColorAttachment);
  }
  else
  {
    GLuint texture_framebuffer;
    glGenFramebuffers(1, &texture_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, texture_framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment->native_id, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    render_target.native_id = NativeFrameBufferID{texture_framebuffer};
  }
  return {};
}

expected<void, RenderTargetError> RenderTargetCache::unload(dependencies deps, RenderTarget& render_target)
{
  render_target.native_id = NativeFrameBufferID{0};
  return {};
}

expected<RenderTarget, RenderTargetError> RenderTargetCache::generate(dependencies deps, TextureHandle color_attachment)
{
  RenderTarget render_target{.color_attachment = color_attachment, .native_id = NativeFrameBufferID{0}};
  if (auto ok_or_error = reload(deps, render_target); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return render_target;
}

}  // namespace sde::graphics
