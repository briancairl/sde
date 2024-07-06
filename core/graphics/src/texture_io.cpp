// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/image_io.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::TextureShape>::operator()(
  binary_ofarchive& ar,
  const graphics::TextureShape& shape) const
{
  ar << named{"value", shape.value};
}

template <>
void load<binary_ifarchive, graphics::TextureShape>::operator()(binary_ifarchive& ar, graphics::TextureShape& shape)
  const
{
  ar >> named{"value", shape.value};
}

template <>
void save<binary_ofarchive, graphics::TextureOptions>::operator()(
  binary_ofarchive& ar,
  const graphics::TextureOptions& options) const
{
  ar << named{"u_wrapping", options.u_wrapping};
  ar << named{"v_wrapping", options.v_wrapping};
  ar << named{"min_sampling", options.min_sampling};
  ar << named{"mag_sampling", options.mag_sampling};
  ar << named{"flags", options.flags};
}

template <>
void load<binary_ifarchive, graphics::TextureOptions>::operator()(
  binary_ifarchive& ar,
  graphics::TextureOptions& options) const
{
  ar >> named{"u_wrapping", options.u_wrapping};
  ar >> named{"v_wrapping", options.v_wrapping};
  ar >> named{"min_sampling", options.min_sampling};
  ar >> named{"mag_sampling", options.mag_sampling};
  ar >> named{"flags", options.flags};
}

template <>
void load<binary_ifarchive, graphics::TextureCache>::operator()(binary_ifarchive& ar, graphics::TextureCache& cache)
  const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::TextureHandle handle;
    ar >> named{"handle", handle};
    graphics::TypeCode element_type;
    ar >> named{"element_type", element_type};
    graphics::TextureLayout layout;
    ar >> named{"layout", layout};
    graphics::TextureShape shape;
    ar >> named{"shape", shape};
    graphics::TextureOptions options;
    ar >> named{"options", options};
    graphics::ImageHandle source_image;
    ar >> named{"source_image", source_image};
    if (source_image.isValid())
    {
      cache.insert(handle, source_image, options, ResourceLoading::kDeferred);
    }
    else
    {
      cache.insert(handle, element_type, shape, layout, options, ResourceLoading::kDeferred);
    }
  }
}

}  // namespace sde::serial
