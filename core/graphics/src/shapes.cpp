// C++ Standard Library
#include <algorithm>
#include <ostream>

// SDE
#include "sde/graphics/shapes.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, const Line& line)
{
  return os << "{ tail: " << line.tail.transpose() << ", head: " << line.head.transpose() << " }";
}

std::ostream& operator<<(std::ostream& os, const Rect& rect)
{
  return os << "{ min: " << rect.min().transpose() << ", max: " << rect.max().transpose() << " }";
}

std::ostream& operator<<(std::ostream& os, const Quad& quad)
{
  return os << "{ rect: " << quad.rect << ", color: " << quad.color.transpose() << " }";
}

std::ostream& operator<<(std::ostream& os, const TexturedQuad& quad)
{
  return os << "{ rect: " << quad.rect << ", rect_texture: " << quad.rect_texture
            << ", color: " << quad.color.transpose() << " }";
}

std::ostream& operator<<(std::ostream& os, const Circle& circle)
{
  return os << "{ center: " << circle.center.transpose() << ", radius: " << circle.radius
            << ", color: " << circle.color.transpose() << " }";
}

}  // namespace sde::graphics
