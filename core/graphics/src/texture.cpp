// C++ Standard Library
#include <iomanip>
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/texture.hpp"

namespace sde::graphics
{
namespace  // anonymous
{}  // namespace anonymous

std::ostream& operator<<(std::ostream& os, TextureHandle handle) { return os << "{ id: " << handle.id << " }"; }

std::ostream& operator<<(std::ostream& os, const TextureShape& shape)
{
  return os << "{ height: " << shape.height << ", width: " << shape.height << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureInfo& info)
{
  return os << "{ shape: " << info.shape << ", layout: " << info.layout << " }";
}

std::ostream& operator<<(std::ostream& os, TextureWrapping wrapping)
{
  switch (wrapping)
  {
  case TextureWrapping::kClampToBorder:
    return os << "ClampToBorder";
  case TextureWrapping::kRepeat:
    return os << "Repeat";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureSampling sampling)
{
  switch (sampling)
  {
  case TextureSampling::kLinear:
    return os << "Linear";
  case TextureSampling::kNearest:
    return os << "Nearest";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureFlags flags)
{
  return os << std::boolalpha << "{ " << static_cast<bool>(flags.unpack_alignment) << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureOptions& options)
{
  return os << "{ u_wrapping: " << options.u_wrapping << ", v_wrapping: " << options.v_wrapping
            << ", min_sampling: " << options.min_sampling << ", mag_sampling: " << options.mag_sampling
            << ", flags: " << options.flags << " }";
}

TextureHandle TextureCache::create(const Image& image, const TextureOptions& options) {}

}  // namespace sde::graphics
