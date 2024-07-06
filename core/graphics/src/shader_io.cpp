// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/shader.hpp"
#include "sde/graphics/shader_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::ShaderCache>::operator()(Archive& ar, const graphics::ShaderCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"path", info.path};
  }
}


template <typename Archive>
void load<Archive, graphics::ShaderCache>::operator()(Archive& ar, graphics::ShaderCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::ShaderHandle handle;
    ar >> named{"handle", handle};
    asset::path path;
    ar >> named{"path", path};
    cache.insert(handle, path, ResourceLoading::kDeferred);
  }
}

template struct save<binary_ofarchive, graphics::ShaderCache>;
template struct load<binary_ifarchive, graphics::ShaderCache>;

}  // namespace sde::serial
