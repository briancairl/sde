// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/render_target.hpp"
#include "sde/graphics/render_target_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::RenderTargetHandle>::operator()(Archive& ar, const graphics::RenderTargetHandle& handle)
  const
{
  ar << handle.fundemental();
}


template <typename Archive>
void load<Archive, graphics::RenderTargetHandle>::operator()(Archive& ar, graphics::RenderTargetHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename Archive>
void save<Archive, graphics::RenderTargetCache>::operator()(Archive& ar, const graphics::RenderTargetCache& cache) const
{
  ar << cache.fundemental();
}


template <typename Archive>
void load<Archive, graphics::RenderTargetCache>::operator()(Archive& ar, graphics::RenderTargetCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::RenderTargetHandle>;
template struct load<binary_ifarchive, graphics::RenderTargetHandle>;
template struct save<binary_ofarchive, graphics::RenderTargetCache>;
template struct load<binary_ifarchive, graphics::RenderTargetCache>;

}  // namespace sde::serial
