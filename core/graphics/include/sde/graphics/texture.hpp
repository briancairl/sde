/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <unordered_map>

// SDE
#include "sde/expected.hpp"

// SDE
#include "sde/geometry_types.hpp"
#include "sde/graphics/image_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

/**
 * @brief Texture channel layout specifier
 */
enum class TextureLayout : std::uint8_t
{
  kR,
  kRG,
  kRGB,
  kRGBA
};

std::ostream& operator<<(std::ostream& os, TextureLayout layout);

/*
 * @brief Image loading options
 */
struct TextureFlags
{
  std::uint8_t unpack_alignment : 1;
  std::uint8_t generate_mip_map : 1;
};

std::ostream& operator<<(std::ostream& os, TextureFlags flags);

enum class TextureWrapping : std::uint8_t
{
  kClampToBorder,
  kClampToEdge,
  kRepeat
};

std::ostream& operator<<(std::ostream& os, TextureWrapping wrapping);

enum class TextureSampling : std::uint8_t
{
  kLinear,
  kNearest
};

std::ostream& operator<<(std::ostream& os, TextureSampling sampling);

/**
 * @brief Texture creation options
 */
struct TextureOptions
{
  TextureWrapping u_wrapping = TextureWrapping::kClampToBorder;
  TextureWrapping v_wrapping = TextureWrapping::kClampToBorder;

  TextureSampling min_sampling = TextureSampling::kNearest;
  TextureSampling mag_sampling = TextureSampling::kNearest;

  TextureFlags flags = {0};
};

std::ostream& operator<<(std::ostream& os, const TextureOptions& options);

struct TextureShape
{
  Vec2i value = {};
  auto width() const { return value.x(); }
  auto height() const { return value.y(); }
  auto texels() const { return value.size(); }
};

std::ostream& operator<<(std::ostream& os, const TextureShape& shape);

struct TextureNativeDeleter
{
  void operator()(native_texture_id_t id) const;
};

using TextureNativeID = UniqueResource<native_texture_id_t, TextureNativeDeleter>;

struct TextureInfo
{
  TextureLayout layout;
  TextureShape shape;
  TextureOptions options;
  TextureNativeID native_id;
};

std::ostream& operator<<(std::ostream& os, const TextureInfo& info);

enum class TextureError
{
  kElementAlreadyExists,
  kInvalidHandle,
  kInvalidDimensions,
  kInvalidDataValue,
  kInvalidDataLength,
  kBackendCreationFailure,
  kBackendTransferFailure,
  kBackendMipMapGenerationFailure,
  kReplaceAreaEmpty,
  kReplaceAreaOutOfBounds,
};

std::ostream& operator<<(std::ostream& os, TextureError error);

class TextureCache;

}  // namespace sde::graphics

namespace sde
{

template <> struct ResourceCacheTypes<graphics::TextureCache>
{
  using error_type = graphics::TextureError;
  using handle_type = graphics::TextureHandle;
  using value_type = graphics::TextureInfo;
};

}  // namespace sde

namespace sde::graphics
{

class TextureCache : public ResourceCache<TextureCache>
{
  friend class ResourceCache<TextureCache>;

public:
  TextureCache() = default;
  ~TextureCache() = default;

private:
  expected<TextureInfo, TextureError> generate(const Image& image, const TextureOptions& options = {});

  template <typename DataT>
  expected<TextureInfo, TextureError>
  generate(View<const DataT> data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {});

  template <typename DataT>
  expected<TextureInfo, TextureError> generate(
    TypeTag<const DataT> /*_*/,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});
};


template <typename DataT>
expected<void, TextureError> replace(const TextureInfo& texture_info, View<const DataT> data, const Bounds2i& area);

template <typename DataT> expected<void, TextureError> replace(const TextureInfo& texture_info, View<const DataT> data)
{
  return replace(texture_info, data, Bounds2i{Vec2i{0, 0}, texture_info.shape.value});
}

}  // namespace sde::graphics
