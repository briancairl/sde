/**
 * @copyright 2024-present Brian Cairl
 *
 * @file image.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/geometry.hpp"
#include "sde/graphics/image_fwd.hpp"
#include "sde/graphics/image_handle.hpp"
#include "sde/graphics/image_ref.hpp"
#include "sde/graphics/typecode.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/unique_resource.hpp"
#include "sde/unordered_map.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, ImageChannels channels);

/**
 * @brief Returns number of channels associated with a particular channel layout
 */
constexpr std::size_t to_channel_count(ImageChannels channels)
{
  switch (channels)
  {
  case ImageChannels::kDefault:
    break;
  case ImageChannels::kGrey:
    return 1;
  case ImageChannels::kGreyA:
    return 2;
  case ImageChannels::kRGB:
    return 3;
  case ImageChannels::kRGBA:
    return 4;
  }
  return 0;
}

/**
 * @brief Returns  channel layout associated with a channel count
 */
inline ImageChannels from_channel_count(std::size_t count)
{
  switch (count)
  {
  case 1:
    return ImageChannels::kGrey;
  case 2:
    return ImageChannels::kGreyA;
  case 3:
    return ImageChannels::kRGB;
  case 4:
    return ImageChannels::kRGBA;
  }
  return ImageChannels::kDefault;
}

/**
 * @brief Image load options
 */
struct ImageOptions : Resource<ImageOptions>
{
  /// Image channel loading options
  ImageChannels channels = ImageChannels::kDefault;
  /// Image channel loading options
  TypeCode element_type = TypeCode::kUInt8;
  /// Image was flipped vertically on load
  bool flip_vertically = false;

  auto field_list()
  {
    return FieldList(
      Field{"channels", channels}, Field{"element_type", element_type}, Field{"flip_vertically", flip_vertically});
  }
};

/**
 * @brief Image dimensions
 */
struct ImageShape : Resource<ImageShape>
{
  Vec2i value = {};
  auto width() const { return value.x(); }
  auto height() const { return value.y(); }
  auto pixels() const { return value.x() * value.y(); }

  auto field_list() { return FieldList(Field{"value", value}); }
};

/**
 * @brief Error codes pertaining to Image loading
 */
enum class ImageError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kAssetNotFound,
  kAssetInvalid,
  kImageNotFound,
  kUnsupportedBitDepth,
};

std::ostream& operator<<(std::ostream& os, ImageError error);

struct ImageDataBufferDeleter
{
  void operator()(void* data) const;
};

using ImageDataBuffer = UniqueResource<void*, ImageDataBufferDeleter>;

struct Image : Resource<Image>
{
  /// Path to image
  asset::path path = {};
  /// Image load options
  ImageOptions options = {};
  /// Image shape
  ImageShape shape = {};
  /// Image data (in memory)
  ImageDataBuffer data_buffer = ImageDataBuffer{nullptr};

  auto field_list()
  {
    return FieldList(
      Field{"path", path}, Field{"options", options}, Field{"shape", shape}, _Stub{"data_buffer", data_buffer});
  }

  /**
   * @brief Returns image channel count
   */
  [[nodiscard]] std::size_t getChannelCount() const { return to_channel_count(options.channels); }

  /**
   * @brief Returns size of single pixel, in bytes
   */
  [[nodiscard]] std::size_t getPixelSizeInBytes() const { return getChannelCount() * byte_count(options.element_type); }

  /**
   * @brief Returns total size of image in bytes
   */
  [[nodiscard]] std::size_t getTotalSizeInBytes() const { return shape.pixels() * getPixelSizeInBytes(); }

  /**
   * @brief Returns pointer to image data
   */
  [[nodiscard]] auto data() const
  {
    return View<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(data_buffer.value()), getTotalSizeInBytes()};
  }

  /**
   * @brief Returns pointer to image data
   */
  [[nodiscard]] auto ref() const
  {
    return ImageRef{
      .channels = options.channels,
      .element_type = options.element_type,
      .width = shape.width(),
      .height = shape.height(),
      .data = data_buffer.value()};
  }
};

class ImageCache : public ResourceCache<ImageCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  ImageHandle to_handle(const asset::path& path) const;

private:
  sde::unordered_map<asset::path, ImageHandle> path_to_image_handle_;

  static expected<void, ImageError> reload(dependencies deps, Image& image);
  static expected<void, ImageError> unload(dependencies deps, Image& image);

  expected<Image, ImageError>
  generate(dependencies deps, const asset::path& image_path, const ImageOptions& options = {});

  void when_created(dependencies deps, ImageHandle handle, const Image* image);
  void when_removed(dependencies deps, ImageHandle handle, const Image* image);
};

}  // namespace sde::graphics

namespace sde
{

template <> struct Hasher<graphics::ImageOptions> : ResourceHasher
{};

template <> struct Hasher<graphics::Image> : ResourceHasher
{};
}  // namespace sde