// C++ Standard Library
#include <iomanip>
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/texture_2D.hpp"

namespace sde::graphics
{
namespace  // anonymous
{}  // namespace anonymous

std::ostream& operator<<(std::ostream& os, Texture2DHandle handle) { return os << "{ id: " << handle.id << " }"; }

std::ostream& operator<<(std::ostream& os, const Texture2DShape& shape)
{
  return os << "{ height: " << shape.height << ", width: " << shape.height << " }";
}

std::ostream& operator<<(std::ostream& os, const Texture2DInfo& info)
{
  return os << "{ shape: " << info.shape << ", layout: " << info.layout << " }";
}

std::ostream& operator<<(std::ostream& os, Texture2DWrapping wrapping)
{
  switch (wrapping)
  {
  case Texture2DWrapping::kClampToBorder:
    return os << "ClampToBorder";
  case Texture2DWrapping::kRepeat:
    return os << "Repeat";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, Texture2DSampling sampling)
{
  switch (sampling)
  {
  case Texture2DSampling::kLinear:
    return os << "Linear";
  case Texture2DSampling::kNearest:
    return os << "Nearest";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, Texture2DFlags flags)
{
  return os << std::boolalpha << "{ " << static_cast<bool>(flags.unpack_alignment) << " }";
}

std::ostream& operator<<(std::ostream& os, const Texture2DOptions& options)
{
  return os << "{ u_wrapping: " << options.u_wrapping << ", v_wrapping: " << options.v_wrapping
            << ", min_sampling: " << options.min_sampling << ", mag_sampling: " << options.mag_sampling
            << ", flags: " << options.flags << " }";
}

Texture2DHandle Texture2DCache::create(const Image& image, const Texture2DOptions& options) {}

}  // namespace sde::graphics
