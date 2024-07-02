// SDE
#include "sde/graphics/font_io.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::FontCacheWithAssets>::operator()(
  binary_ofarchive& ar,
  const graphics::FontCacheWithAssets& cache) const
{
  ar << named{"element_count", cache.handles().size()};
  for (const auto& [handle, path] : cache.handles())
  {
    ar << named{"handle", handle};
    ar << named{"path", path};
  }
}


template <>
void load<binary_ifarchive, graphics::FontCacheWithAssets>::operator()(
  binary_ifarchive& ar,
  graphics::FontCacheWithAssets& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::FontHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    cache.load(handle, path);
  }
}

}  // namespace sde::serial
