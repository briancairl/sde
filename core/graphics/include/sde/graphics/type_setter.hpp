/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_setter.hpp
 */
#pragma once

// C++ Standard Library
#include <string_view>

// SDE
#include "sde/graphics/renderer_fwd.hpp"
#include "sde/graphics/shapes.hpp"
#include "sde/graphics/type_set_fwd.hpp"

namespace sde::graphics
{

enum class TextJusificationH
{
  kLeft,
  kCenter,
  kRight,
};

enum class TextJusificationV
{
  kAbove,
  kCenter,
  kBelow,
};

struct TextOptions
{
  float height = 0.1F;
  TextJusificationH justification_x = TextJusificationH::kCenter;
  TextJusificationV justification_y = TextJusificationV::kCenter;
};

class TypeSetter
{
public:
  explicit TypeSetter(const TypeSetHandle& glyphs);

  void draw(
    RenderPass& rp,
    std::string_view text,
    const Vec2f& pos,
    const TextOptions& options,
    const Vec4f& color = Vec4f::Ones()) const;

private:
  TypeSetHandle type_set_handle_;
};

}  // namespace sde::graphics
