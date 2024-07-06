// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/font_io.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::FontCache>::operator()(Archive& ar, const graphics::FontCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"path", info.path};
  }
}


template <typename Archive>
void load<Archive, graphics::FontCache>::operator()(Archive& ar, graphics::FontCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::FontHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    cache.insert(handle, path, ResourceLoading::kDeferred);
  }
}

template struct save<binary_ofarchive, graphics::FontCache>;
template struct load<binary_ifarchive, graphics::FontCache>;

}  // namespace sde::serial
