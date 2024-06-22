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
#include "sde/graphics/typecode.hpp"
#include "sde/view.hpp"

namespace sde::graphics
{

/**
 * @brief Image channel specifier
 */
enum class ImageChannels : std::uint8_t
{
  kDefault,
  kGrey,
  kGreyA,
  kRGB,
  kRGBA
};

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
  TypeCode bit_depth = TypeCode::kUInt8;

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
enum class ImageLoadError
{
  kAssetNotFound,
  kAssetInvalid,
  kUnsupportedBitDepth,
};

std::ostream& operator<<(std::ostream& os, ImageLoadError error);

/**
 * @brief In-memory image data
 */
class Image
{
public:
  ~Image();

  Image(const Image&) = delete;
  Image(Image&& other);

  /**
   * @brief Loads an Image from disk, or returns an ImageLoadError
   */
  [[nodiscard]] static expected<Image, ImageLoadError>
  load(const asset::path& image_path, const ImageOptions& options = {});

  /**
   * @brief Returns true if held resource is valid
   */
  [[nodiscard]] constexpr bool valid() const { return data_ != nullptr; }

  /**
   * @copydoc valid
   */
  [[nodiscard]] constexpr operator bool() const { return valid(); }

  /**
   * @brief Returns image dimensions
   */
  [[nodiscard]] constexpr const ImageShape& shape() const { return shape_; }

  /**
   * @brief Returns image pixel depth
   */
  [[nodiscard]] constexpr TypeCode depth() const { return bit_depth_; }

  /**
   * @brief Returns image channel count
   */
  [[nodiscard]] constexpr ImageChannels channels() const { return channels_; }

  /**
   * @brief Returns image channel count
   */
  [[nodiscard]] constexpr std::size_t getChannelCount() const { return to_channel_count(channels_); }

  /**
   * @brief Returns size of single pixel, in bytes
   */
  [[nodiscard]] constexpr std::size_t getPixelSizeInBytes() const { return getChannelCount() * byte_count(bit_depth_); }

  /**
   * @brief Returns total size of image in bytes
   */
  [[nodiscard]] constexpr std::size_t getTotalSizeInBytes() const { return shape_.pixels() * getPixelSizeInBytes(); }

  /**
   * @brief Returns pointer to image data
   */
  [[nodiscard]] auto data() const
  {
    return View<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(data_), getTotalSizeInBytes()};
  }

private:
  Image(const ImageShape& shape, ImageChannels channels, TypeCode bit_depth, void* data);

  /// Image size
  ImageShape shape_;
  /// Image channel layout
  ImageChannels channels_;
  /// Image pixel depth
  TypeCode bit_depth_;
  /// Image data buffer pointer
  void* data_;
};

std::ostream& operator<<(std::ostream& os, const Image& error);

}  // namespace sde::graphics
