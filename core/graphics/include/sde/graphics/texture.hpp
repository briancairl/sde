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

struct TextureInfo
{
  TextureLayout layout;
  TextureShape shape;
  TextureOptions options;
  native_texture_id_t native_id;
};

std::ostream& operator<<(std::ostream& os, const TextureInfo& info);

enum class TextureError
{
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

class TextureCache
{
public:
  TextureCache() = default;

  ~TextureCache();

  bool remove(const TextureHandle& index);

  expected<TextureHandle, TextureError> upload(const Image& image, const TextureOptions& options = {})
  {
    const auto texture = newTextureHandle();
    const auto ok_or_error = transfer(texture, image, options);
    if (ok_or_error.has_value())
    {
      last_texture_handle_ = texture;
      return texture;
    }
    return make_unexpected(ok_or_error.error());
  }

  template <typename DataT>
  expected<TextureHandle, TextureError>
  upload(View<const DataT> data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {})
  {
    const auto texture = newTextureHandle();
    const auto ok_or_error = transfer(texture, data, shape, options);
    if (ok_or_error.has_value())
    {
      last_texture_handle_ = texture;
      return texture;
    }
    return make_unexpected(ok_or_error.error());
  }

  template <typename DataT>
  expected<TextureHandle, TextureError>
  create(const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {})
  {
    const auto texture = newTextureHandle();
    const auto ok_or_error = allocate<DataT>(texture, shape, layout, options);
    if (ok_or_error.has_value())
    {
      last_texture_handle_ = texture;
      return texture;
    }
    return make_unexpected(ok_or_error.error());
  }

  const TextureInfo* get(TextureHandle texture) const;

private:
  using TextureCacheMap = std::unordered_map<TextureHandle, TextureInfo, ResourceHandleHash>;

  expected<void, TextureError> transfer(TextureHandle, const Image& image, const TextureOptions& options);

  template <typename DataT>
  expected<void, TextureError> transfer(
    TextureHandle texture,
    View<const DataT> data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  template <typename DataT>
  expected<void, TextureError>
  allocate(TextureHandle texture, const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {});

  TextureHandle last_texture_handle_ = TextureHandle::null();

  TextureCacheMap textures_;

  TextureHandle newTextureHandle() const { return TextureHandle{last_texture_handle_.id() + 1UL}; }
};


template <typename DataT>
expected<void, TextureError> replace(const TextureInfo& texture_info, View<const DataT> data, const Bounds2i& area);

template <typename DataT> expected<void, TextureError> replace(const TextureInfo& texture_info, View<const DataT> data)
{
  return replace(texture_info, data, Bounds2i{Vec2i{0, 0}, texture_info.shape.value});
}

}  // namespace sde::graphics
