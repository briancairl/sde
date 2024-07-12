// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/shader.hpp"
#include "sde/graphics/shader_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::ShaderHandle>::operator()(Archive& ar, const graphics::ShaderHandle& handle) const
{
  ar << handle.fundemental();
}


template <typename Archive>
void load<Archive, graphics::ShaderHandle>::operator()(Archive& ar, graphics::ShaderHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename Archive>
void save<Archive, graphics::ShaderCache>::operator()(Archive& ar, const graphics::ShaderCache& cache) const
{
  ar << cache.fundemental();
}


template <typename Archive>
void load<Archive, graphics::ShaderCache>::operator()(Archive& ar, graphics::ShaderCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::ShaderHandle>;
template struct load<binary_ifarchive, graphics::ShaderHandle>;
template struct save<binary_ofarchive, graphics::ShaderCache>;
template struct load<binary_ifarchive, graphics::ShaderCache>;

}  // namespace sde::serial
