// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/image.hpp"
#include "sde/graphics/image_io.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::ImageOptions>::operator()(Archive& ar, const graphics::ImageOptions& options) const
{
  ar << named{"channels", options.channels};
  ar << named{"element_type", options.element_type};
  ar << named{"flags", options.flags};
}

template <typename Archive>
void load<Archive, graphics::ImageOptions>::operator()(Archive& ar, graphics::ImageOptions& options) const
{
  ar >> named{"channels", options.channels};
  ar >> named{"element_type", options.element_type};
  ar >> named{"flags", options.flags};
}

template <typename Archive>
void save<Archive, graphics::ImageCache>::operator()(Archive& ar, const graphics::ImageCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"path", info.path};
    ar << named{"options", info.options};
  }
}


template <typename Archive>
void load<Archive, graphics::ImageCache>::operator()(Archive& ar, graphics::ImageCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::ImageHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    graphics::ImageOptions options;
    ar >> named{"options", options};
    SDE_ASSERT_OK(cache.insert(handle, path, options, ResourceLoading::kDeferred));
  }
}

template struct save<binary_ofarchive, graphics::ImageOptions>;
template struct load<binary_ifarchive, graphics::ImageOptions>;
template struct save<binary_ofarchive, graphics::ImageCache>;
template struct load<binary_ifarchive, graphics::ImageCache>;

}  // namespace sde::serial
