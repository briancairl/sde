// C++ Standard Library
#include <ostream>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/font_io.hpp"
#include "sde/graphics/type_set_io.hpp"
#include "sde/logging.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <>
void save<binary_ofarchive, graphics::TypeSetCache>::operator()(
  binary_ofarchive& ar,
  const graphics::TypeSetCache& cache) const
{
  ar << named{"element_count", cache.size()};
  for (const auto& [handle, info] : cache)
  {
    ar << named{"handle", handle};
    ar << named{"font", info.font};
    ar << named{"options", info.options};
  }
}

template <>
void load<binary_ifarchive, graphics::TypeSetCache>::operator()(binary_ifarchive& ar, graphics::TypeSetCache& cache)
  const
{
  std::size_t element_count{0};
  ar >> named{"element_count", element_count};
  for (std::size_t element_idx = 0; element_idx < element_count; ++element_idx)
  {
    graphics::TypeSetHandle handle;
    ar >> named{"handle", handle};
    graphics::FontHandle font;
    ar >> named{"font", font};
    graphics::TypeSetOptions options;
    ar >> named{"options", options};
    cache.insert(handle, font, options);
  }
}

}  // namespace sde::serial
