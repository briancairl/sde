/**
 * @copyright 2024-present Brian Cairl
 *
 * @file render_target.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <variant>

// SDE
#include "sde/expected.hpp"
#include "sde/geometry_types.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/graphics/window_handle.hpp"
#include "sde/resource_handle.hpp"

namespace sde::graphics
{
class RenderTargetActive;

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

  RenderTarget(RenderTarget&& other);

  static expected<RenderTarget, RenderTargetError> create(const WindowHandle& window);

  static expected<RenderTarget, RenderTargetError>
  create(const TextureHandle& texture, const TextureCache& texture_cache);

  RenderTargetHandle handle() const
  {
    return std::holds_alternative<RenderTargetHandle>(target_) ? std::get<RenderTargetHandle>(target_)
                                                               : RenderTargetHandle::null();
  }

  void refresh();

  void refresh(const Vec4f& clear_color);

  Vec2i getLastSize() const { return viewport_size_; }

  float getLastAspectRatio() const { return toAspectRatio(viewport_size_); }

  static float toAspectRatio(const Vec2i viewport_size)
  {
    return static_cast<float>(viewport_size.x()) / static_cast<float>(viewport_size.y());
  }

private:
  void activate();

  friend class RenderTargetActive;

  explicit RenderTarget(WindowHandle window);
  RenderTarget(RenderTargetHandle frame_buffer, Vec2i size);

  std::variant<WindowHandle, RenderTargetHandle> target_;

  Vec2i viewport_size_;
};

}  // namespace sde::graphics
