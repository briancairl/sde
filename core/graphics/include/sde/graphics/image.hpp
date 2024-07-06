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
#include "sde/geometry_types.hpp"
#include "sde/graphics/image_fwd.hpp"
#include "sde/graphics/image_handle.hpp"
#include "sde/graphics/image_ref.hpp"
#include "sde/graphics/typecode.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_wrapper.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, ImageChannels channels);

/**
 * @brief Returns number of channels associated with a particular channel layout
 */
inline std::size_t to_channel_count(ImageChannels channels)
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
 * @brief Image loading options
 */
struct ImageLoadFlags
{
  std::uint8_t flip_vertically : 1;
};

std::ostream& operator<<(std::ostream& os, ImageLoadFlags flags);

/**
 * @brief Image load options
 */
struct ImageOptions
{
  /// Image channel loading options
  ImageChannels channels = ImageChannels::kDefault;
  /// Image channel loading options
  TypeCode element_type = TypeCode::kUInt8;
  /// On-load option flags
  ImageLoadFlags flags = {0};
};

std::ostream& operator<<(std::ostream& os, const ImageOptions& error);

/**
 * @brief Image dimensions
 */
struct ImageShape
{
  Vec2i value = {};
  auto width() const { return value.x(); }
  auto height() const { return value.y(); }
  auto pixels() const { return value.x() * value.y(); }
};

std::ostream& operator<<(std::ostream& os, const ImageShape& error);

/**
 * @brief Error codes pertaining to Image loading
 */
enum class ImageError
{
  kAssetNotFound,
  kAssetInvalid,
  kElementAlreadyExists,
  kImageNotFound,
  kUnsupportedBitDepth,
};

std::ostream& operator<<(std::ostream& os, ImageError error);

struct ImageDataBufferDeleter
{
  void operator()(void* data) const;
};

using ImageDataBuffer = UniqueResource<void*, ImageDataBufferDeleter>;

struct ImageInfo
{
  /// Path to image
  asset::path path;
  /// Image load options
  ImageOptions options;
  /// Image shape
  ImageShape shape;
  /// Image data (in memory)
  ImageDataBuffer data_buffer;

  /**
   * @brief Returns image channel count
   */
  [[nodiscard]] constexpr std::size_t getChannelCount() const { return to_channel_count(options.channels); }

  /**
   * @brief Returns size of single pixel, in bytes
   */
  [[nodiscard]] constexpr std::size_t getPixelSizeInBytes() const
  {
    return getChannelCount() * byte_count(options.element_type);
  }

  /**
   * @brief Returns total size of image in bytes
   */
  [[nodiscard]] constexpr std::size_t getTotalSizeInBytes() const { return shape.pixels() * getPixelSizeInBytes(); }

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

}  // namespace sde::graphics

namespace sde
{

template <> struct ResourceCacheTypes<graphics::ImageCache>
{
  using error_type = graphics::ImageError;
  using handle_type = graphics::ImageHandle;
  using value_type = graphics::ImageInfo;
};

}  // namespace sde

namespace sde::graphics
{

class ImageCache : public ResourceCache<ImageCache>
{
  friend cache_base;

private:
  static expected<void, ImageError> reload(ImageInfo& image);
  static expected<void, ImageError> unload(ImageInfo& image);
  expected<ImageInfo, ImageError> generate(
    const asset::path& image_path,
    const ImageOptions& options = {},
    ResourceLoading loading = ResourceLoading::kImmediate);
};

}  // namespace sde::graphics