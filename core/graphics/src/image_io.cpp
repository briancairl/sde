// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/image.hpp"
#include "sde/graphics/image_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::ImageOptions>::operator()(Archive& ar, const graphics::ImageOptions& options) const
{
  ar << options.fundemental();
}

template <typename Archive>
void load<Archive, graphics::ImageOptions>::operator()(Archive& ar, graphics::ImageOptions& options) const
{
  ar >> options.fundemental();
}

template <typename Archive>
void save<Archive, graphics::ImageHandle>::operator()(Archive& ar, const graphics::ImageHandle& handle) const
{
  ar << handle.fundemental();
}

template <typename Archive>
void load<Archive, graphics::ImageHandle>::operator()(Archive& ar, graphics::ImageHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename Archive>
void save<Archive, graphics::ImageCache>::operator()(Archive& ar, const graphics::ImageCache& cache) const
{
  ar << cache.fundemental();
}

template <typename Archive>
void load<Archive, graphics::ImageCache>::operator()(Archive& ar, graphics::ImageCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::ImageOptions>;
template struct load<binary_ifarchive, graphics::ImageOptions>;
template struct save<binary_ofarchive, graphics::ImageHandle>;
template struct load<binary_ifarchive, graphics::ImageHandle>;
template struct save<binary_ofarchive, graphics::ImageCache>;
template struct load<binary_ifarchive, graphics::ImageCache>;

}  // namespace sde::serial
