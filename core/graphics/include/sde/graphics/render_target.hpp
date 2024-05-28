/**
 * @copyright 2024-present Brian Cairl
 *
 * @file renderer.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <variant>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/resource_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/graphics/window_handle.hpp"

namespace sde::graphics
{

struct RenderTargetHandle : ResourceHandle<RenderTargetHandle>
{
  RenderTargetHandle() = default;
  explicit RenderTargetHandle(id_type id) : ResourceHandle<RenderTargetHandle>{id} {}
};

enum class RenderTargetError
{
  kInvalidTexture,
  kInvalidWindow,
};

std::ostream& operator<<(std::ostream& os, RenderTargetError error);

class RenderTarget
{
public:
  ~RenderTarget();

  static expected<RenderTarget, RenderTargetError> create(const WindowHandle& window);
  static expected<RenderTarget, RenderTargetError>
  create(const TextureHandle& texture, const TextureCache& texture_cache);

  RenderTargetHandle handle() const
  {
    return std::holds_alternative<RenderTargetHandle>(target_) ? std::get<RenderTargetHandle>(target_)
                                                               : RenderTargetHandle::null();
  }

  void activate();

  Vec2i refresh(const Vec4f& clear_color = Vec4f::Zero());

  Vec2i getLastSize() const { return viewport_size_; }

private:
  explicit RenderTarget(WindowHandle window);
  RenderTarget(RenderTargetHandle frame_buffer, Vec2i size);

  std::variant<WindowHandle, RenderTargetHandle> target_;
  Vec2i viewport_size_;
};

}  // namespace sde::graphics
