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

template <typename Archive>
void save<Archive, graphics::TextureShape>::operator()(Archive& ar, const graphics::TextureShape& shape) const
{
  ar << named{"value", shape.value};
}

template <typename Archive>
void load<Archive, graphics::TextureShape>::operator()(Archive& ar, graphics::TextureShape& shape) const
{
  ar >> named{"value", shape.value};
}

template <typename Archive>
void save<Archive, graphics::TextureOptions>::operator()(Archive& ar, const graphics::TextureOptions& options) const
{
  ar << named{"u_wrapping", options.u_wrapping};
  ar << named{"v_wrapping", options.v_wrapping};
  ar << named{"min_sampling", options.min_sampling};
  ar << named{"mag_sampling", options.mag_sampling};
  ar << named{"flags", options.flags};
}

template <typename Archive>
void load<Archive, graphics::TextureOptions>::operator()(Archive& ar, graphics::TextureOptions& options) const
{
  ar >> named{"u_wrapping", options.u_wrapping};
  ar >> named{"v_wrapping", options.v_wrapping};
  ar >> named{"min_sampling", options.min_sampling};
  ar >> named{"mag_sampling", options.mag_sampling};
  ar >> named{"flags", options.flags};
}

template <typename Archive>
void save<Archive, graphics::TextureCache>::operator()(Archive& ar, const graphics::TextureCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"element_type", info.element_type};
    ar << named{"layout", info.layout};
    ar << named{"shape", info.shape};
    ar << named{"options", info.options};
    ar << named{"source_image", info.source_image};
  }
}

template <typename Archive>
void load<Archive, graphics::TextureCache>::operator()(Archive& ar, graphics::TextureCache& cache) const
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
    SDE_ASSERT_OK(cache.insert(handle, element_type, shape, layout, options, source_image, ResourceLoading::kDeferred));
  }
}

template struct save<binary_ofarchive, graphics::TextureShape>;
template struct load<binary_ifarchive, graphics::TextureShape>;
template struct save<binary_ofarchive, graphics::TextureOptions>;
template struct load<binary_ifarchive, graphics::TextureOptions>;
template struct save<binary_ofarchive, graphics::TextureCache>;
template struct load<binary_ifarchive, graphics::TextureCache>;

}  // namespace sde::serial
