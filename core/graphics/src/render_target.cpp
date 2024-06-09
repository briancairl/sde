// C++ Standard Library
#include <ostream>

// Backend
#include "opengl.inl"

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// SDE
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/window.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, RenderTargetError error)
{
  switch (error)
  {
  case RenderTargetError::kInvalidTexture:
    return os << "InvalidTexture";
  case RenderTargetError::kInvalidWindow:
    return os << "InvalidWindow";
  }
  return os;
}

RenderTarget::RenderTarget(RenderTarget&& other) :
    target_{std::move(other.target_)}, viewport_size_{std::move(other.viewport_size_)}
{}

RenderTarget::RenderTarget(WindowNativeHandle window) : target_{window}, viewport_size_{Vec2i::Zero()} {}

RenderTarget::RenderTarget(RenderTargetHandle frame_buffer, Vec2i size) : target_{frame_buffer}, viewport_size_{size} {}

RenderTarget::~RenderTarget()
{
  if (handle().isValid())
  {
    GLuint texture_framebuffer = handle().id();
    glDeleteFramebuffers(1, &texture_framebuffer);
  }
}

expected<RenderTarget, RenderTargetError> RenderTarget::create(const Window& window)
{
  if (window.isNull())
  {
    return unexpected<RenderTargetError>{RenderTargetError::kInvalidWindow};
  }
  return RenderTarget{window.value()};
}

expected<RenderTarget, RenderTargetError>
RenderTarget::create(const TextureHandle& texture, const TextureCache& texture_cache)
{
  if (texture.isNull())
  {
    return unexpected<RenderTargetError>{RenderTargetError::kInvalidTexture};
  }

  const auto* texture_info = texture_cache.get_if(texture);

  if (texture_info == nullptr)
  {
    return unexpected<RenderTargetError>{RenderTargetError::kInvalidTexture};
  }

  GLuint texture_framebuffer;
  glGenFramebuffers(1, &texture_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, texture_framebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_info->native_id, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return RenderTarget{RenderTargetHandle{texture_framebuffer}, texture_info->shape.value};
}

void RenderTarget::activate() { glBindFramebuffer(GL_FRAMEBUFFER, handle().id()); }

void RenderTarget::refresh()
{
  if (const WindowNativeHandle* window_handle_native = std::get_if<WindowNativeHandle>(&target_);
      window_handle_native != nullptr)
  {
    glfwGetFramebufferSize(
      reinterpret_cast<GLFWwindow*>(*window_handle_native), (viewport_size_.data() + 0), (viewport_size_.data() + 1));
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  else
  {
    glBindFramebuffer(GL_FRAMEBUFFER, std::get<RenderTargetHandle>(target_).id());
  }
  glViewport(0, 0, viewport_size_.x(), viewport_size_.y());
}

void RenderTarget::refresh(const Vec4f& color)
{
  refresh();
  glClearColor(color[0], color[1], color[2], color[3]);
  glClear(GL_COLOR_BUFFER_BIT);
}


}  // namespace sde::graphics
