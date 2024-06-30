// SDE
#include "sde/graphics/texture_io.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::TextureCacheWithAssets>::operator()(
  binary_ofarchive& ar,
  const graphics::TextureCacheWithAssets& cache) const
{
  ar << named{"element_count", cache.handles().size()};
  for (const auto& [handle, path] : cache.handles())
  {
    ar << named{"handle", handle};
    ar << named{"path", path};
    const auto* texture_info = cache.get_if(handle);
    SDE_ASSERT_NE(texture_info, nullptr);
    ar << named{"options", texture_info->options};
  }
}


template <>
void load<binary_ifarchive, graphics::TextureCacheWithAssets>::operator()(
  binary_ifarchive& ar,
  graphics::TextureCacheWithAssets& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::TextureHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    graphics::TextureOptions options;
    ar >> named{"options", options};
    cache.load(handle, path, options);
  }
}

}  // namespace sde::serial
