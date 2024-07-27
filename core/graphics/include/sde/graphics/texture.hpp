/**
 * @copyright 2024-present Brian Cairl
 *
 * @file texture.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>

// SDE
#include "sde/expected.hpp"

// SDE
#include "sde/asset.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/image_fwd.hpp"
#include "sde/graphics/image_handle.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/texture_handle.hpp"
#include "sde/graphics/typecode.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"
#include "sde/type.hpp"
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
struct TextureOptions : Resource<TextureOptions>
{
  TextureWrapping u_wrapping = TextureWrapping::kClampToBorder;
  TextureWrapping v_wrapping = TextureWrapping::kClampToBorder;

  TextureSampling min_sampling = TextureSampling::kNearest;
  TextureSampling mag_sampling = TextureSampling::kNearest;

  bool unpack_alignment = false;
  bool generate_mip_map = false;

  auto field_list()
  {
    return FieldList(
      (Field{"u_wrapping", u_wrapping}),
      (Field{"v_wrapping", v_wrapping}),
      (Field{"min_sampling", min_sampling}),
      (Field{"mag_sampling", mag_sampling}),
      (Field{"unpack_alignment", unpack_alignment}),
      (Field{"generate_mip_map", generate_mip_map}));
  }
};

struct TextureShape : Resource<TextureShape>
{
  Vec2i value = {};

  float aspect() const { return static_cast<float>(value.y()) / static_cast<float>(value.x()); }
  auto width() const { return value.x(); }
  auto height() const { return value.y(); }
  auto texels() const { return value.size(); }

  auto field_list() { return FieldList((Field{"value", value})); }
};

struct TextureNativeDeleter
{
  void operator()(native_texture_id_t id) const;
};

using NativeTextureID = UniqueResource<native_texture_id_t, TextureNativeDeleter>;

struct Texture : Resource<Texture>
{
  ImageHandle source_image = ImageHandle::null();
  TypeCode element_type = TypeCode::kUInt8;
  TextureLayout layout = TextureLayout::kR;
  TextureShape shape = {};
  TextureOptions options = {};
  NativeTextureID native_id = NativeTextureID{0};

  auto field_list()
  {
    return FieldList(
      (Field{"source_image", source_image}),
      (Field{"element_type", element_type}),
      (Field{"layout", layout}),
      (Field{"shape", shape}),
      (Field{"options", options}),
      (_Stub{"native_id", native_id}));
  }
};

bool operator==(const Texture& lhs, const Texture& rhs);

enum class TextureError
{
  kElementAlreadyExists,
  kTextureNotFound,
  kInvalidSourceImage,
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

}  // namespace sde::graphics

namespace sde
{

template <> struct Hasher<graphics::TextureOptions> : ResourceHasher
{};

template <> struct Hasher<graphics::TextureShape> : ResourceHasher
{};

template <> struct Hasher<graphics::Texture> : ResourceHasher
{};

template <> struct ResourceCacheTypes<graphics::TextureCache>
{
  using error_type = graphics::TextureError;
  using handle_type = graphics::TextureHandle;
  using value_type = graphics::Texture;
};

}  // namespace sde

namespace sde::graphics
{

template <typename DataT>
expected<void, TextureError> replace(const Texture& texture_info, View<const DataT> data, const Bounds2i& area);

template <typename DataT> expected<void, TextureError> replace(const Texture& texture_info, View<const DataT> data)
{
  return replace(texture_info, data, Bounds2i{Vec2i{0, 0}, texture_info.shape.value});
}

class TextureCache : public ResourceCache<TextureCache>
{
  friend fundemental_type;

public:
  explicit TextureCache(ImageCache& images);

private:
  ImageCache* images_;

  expected<void, TextureError> reload(Texture& texture);
  static expected<void, TextureError> unload(Texture& texture);

  expected<Texture, TextureError> generate(const asset::path& image_path, const TextureOptions& options = {});

  expected<Texture, TextureError> generate(const ImageHandle& image, const TextureOptions& options = {});

  template <typename DataT>
  expected<Texture, TextureError>
  generate(View<const DataT> data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {});

  expected<Texture, TextureError>
  generate(TypeCode type, const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {});
};

}  // namespace sde::graphics
