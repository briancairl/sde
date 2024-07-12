// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/image_io.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
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
  ar << options.fundemental();
}

template <typename Archive>
void load<Archive, graphics::TextureOptions>::operator()(Archive& ar, graphics::TextureOptions& options) const
{
  ar >> options.fundemental();
}

template <typename Archive>
void save<Archive, graphics::TextureHandle>::operator()(Archive& ar, const graphics::TextureHandle& handle) const
{
  ar << handle.fundemental();
}

template <typename Archive>
void load<Archive, graphics::TextureHandle>::operator()(Archive& ar, graphics::TextureHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename Archive>
void save<Archive, graphics::TextureCache>::operator()(Archive& ar, const graphics::TextureCache& cache) const
{
  ar << cache.fundemental();
}

template <typename Archive>
void load<Archive, graphics::TextureCache>::operator()(Archive& ar, graphics::TextureCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::TextureShape>;
template struct load<binary_ifarchive, graphics::TextureShape>;
template struct save<binary_ofarchive, graphics::TextureOptions>;
template struct load<binary_ifarchive, graphics::TextureOptions>;
template struct save<binary_ofarchive, graphics::TextureHandle>;
template struct load<binary_ifarchive, graphics::TextureHandle>;
template struct save<binary_ofarchive, graphics::TextureCache>;
template struct load<binary_ifarchive, graphics::TextureCache>;

}  // namespace sde::serial
