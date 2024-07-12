// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/font_io.hpp"
#include "sde/graphics/texture_io.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/graphics/type_set_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::TypeSetOptions>::operator()(
  binary_ofarchive& ar,
  const graphics::TypeSetOptions& options) const
{
  ar << named{"height_px", options.height_px};
}

template <>
void load<binary_ifarchive, graphics::TypeSetOptions>::operator()(
  binary_ifarchive& ar,
  graphics::TypeSetOptions& options) const
{
  ar >> named{"height_px", options.height_px};
}

template <>
void save<binary_ofarchive, graphics::TypeSetHandle>::operator()(
  binary_ofarchive& ar,
  const graphics::TypeSetHandle& handle) const
{
  ar << handle.fundemental();
}

template <>
void load<binary_ifarchive, graphics::TypeSetHandle>::operator()(binary_ifarchive& ar, graphics::TypeSetHandle& handle)
  const
{
  ar >> handle.fundemental();
}

template <>
void save<binary_ofarchive, graphics::TypeSetCache>::operator()(
  binary_ofarchive& ar,
  const graphics::TypeSetCache& cache) const
{
  ar << cache.fundemental();
}

template <>
void load<binary_ifarchive, graphics::TypeSetCache>::operator()(binary_ifarchive& ar, graphics::TypeSetCache& cache)
  const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::TypeSetHandle>;
template struct load<binary_ifarchive, graphics::TypeSetHandle>;
template struct save<binary_ofarchive, graphics::TypeSetCache>;
template struct load<binary_ifarchive, graphics::TypeSetCache>;

}  // namespace sde::serial
