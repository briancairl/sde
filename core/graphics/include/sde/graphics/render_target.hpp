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
#include "sde/graphics/resource_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/graphics/window_handle.hpp"

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

  static expected<RenderTarget, RenderTargetError> create(const WindowHandle& window);

  static expected<RenderTarget, RenderTargetError>
  create(const TextureHandle& texture, const TextureCache& texture_cache);

  RenderTargetHandle handle() const
  {
    return std::holds_alternative<RenderTargetHandle>(target_) ? std::get<RenderTargetHandle>(target_)
                                                               : RenderTargetHandle::null();
  }

  Vec2i getLastSize() const { return viewport_size_; }

private:
  void activate();

  Vec2i refresh(const Vec4f& clear_color = Vec4f::Zero());

  friend class RenderTargetActive;

  explicit RenderTarget(WindowHandle window);
  RenderTarget(RenderTargetHandle frame_buffer, Vec2i size);

  std::variant<WindowHandle, RenderTargetHandle> target_;
  Vec2i viewport_size_;
};


class RenderTargetActive
{
public:
  explicit RenderTargetActive(RenderTarget& target, Vec4f clear_color = Vec4f::Zero()) :
      last_active_{nullptr}, clear_color_{clear_color}
  {
    exchange(target);
  }

  RenderTarget* exchange(RenderTarget& target)
  {
    auto* prev_active = last_active_;
    if (auto* next_active = std::addressof(target); last_active_ != next_active)
    {
      next_active->refresh(clear_color_);
      last_active_ = next_active;
    }
    return prev_active;
  }

  const RenderTarget* operator->() const { return last_active_; }

private:
  RenderTargetActive(RenderTargetActive&&) = delete;
  RenderTargetActive(const RenderTargetActive&) = delete;

  RenderTarget* last_active_;
  Vec4f clear_color_;
};

}  // namespace sde::graphics
