// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/font.hpp"
#include "sde/graphics/font_handle.hpp"
#include "sde/graphics/font_io.hpp"
#include "sde/resource_cache_io.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::serial
{

template <typename Archive>
void save<Archive, graphics::FontHandle>::operator()(Archive& ar, const graphics::FontHandle& handle) const
{
  ar << handle.fundemental();
}

template <typename Archive>
void load<Archive, graphics::FontHandle>::operator()(Archive& ar, graphics::FontHandle& handle) const
{
  ar >> handle.fundemental();
}

template <typename Archive>
void save<Archive, graphics::FontCache>::operator()(Archive& ar, const graphics::FontCache& cache) const
{
  ar << cache.fundemental();
}

template <typename Archive>
void load<Archive, graphics::FontCache>::operator()(Archive& ar, graphics::FontCache& cache) const
{
  ar >> cache.fundemental();
}

template struct save<binary_ofarchive, graphics::FontHandle>;
template struct load<binary_ifarchive, graphics::FontHandle>;
template struct save<binary_ofarchive, graphics::FontCache>;
template struct load<binary_ifarchive, graphics::FontCache>;

}  // namespace sde::serial
