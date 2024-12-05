/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_set_fwd.hpp
 */
#pragma once

namespace sde::graphics
{
enum class TypeSetError;
struct Glyph;
struct TypeSetOptions;
struct TypeSetHandle;
struct TypeSet;
class TypeSetCache;
}  // namespace sde::graphics

namespace sde
{
template <> struct ResourceCacheTraits<graphics::TypeSetCache>
{
  using error_type = graphics::TypeSetError;
  using handle_type = graphics::TypeSetHandle;
  using value_type = graphics::TypeSet;
  using dependencies = ResourceDependencies<graphics::TextureCache, graphics::FontCache>;
};
}  // namespace sde