// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/render_target_io.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::RenderTargetCache>::operator()(Archive& ar, const graphics::RenderTargetCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"color_attachment", info.color_attachment};
  }
}


template <typename Archive>
void load<Archive, graphics::RenderTargetCache>::operator()(Archive& ar, graphics::RenderTargetCache& cache) const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::RenderTargetHandle handle;
    ar >> named{"handle", handle};
    graphics::TextureHandle color_attachment;
    ar >> named{"color_attachment", color_attachment};
    SDE_ASSERT_OK(cache.insert(handle, color_attachment, ResourceLoading::kDeferred));
  }
}

template struct save<binary_ofarchive, graphics::RenderTargetCache>;
template struct load<binary_ifarchive, graphics::RenderTargetCache>;

}  // namespace sde::serial
