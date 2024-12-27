// C++ Standard Library
#include <iomanip>
#include <ostream>

// STB
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

// SDE
#include "sde/graphics/image.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{
namespace  // anonymous
{

int to_stbi_enum(ImageChannels mode)
{
  switch (mode)
  {
  case ImageChannels::kDefault:
    return STBI_default;
  case ImageChannels::kGrey:
    return STBI_grey;
  case ImageChannels::kGreyA:
    return STBI_grey_alpha;
  case ImageChannels::kRGB:
    return STBI_rgb;
  case ImageChannels::kRGBA:
    return STBI_rgb_alpha;
  }
  return STBI_default;
}

}  // namespace anonymous

std::ostream& operator<<(std::ostream& os, ImageChannels channels)
{
  switch (channels)
  {
    SDE_OS_ENUM_CASE(ImageChannels::kDefault)
    SDE_OS_ENUM_CASE(ImageChannels::kGrey)
    SDE_OS_ENUM_CASE(ImageChannels::kGreyA)
    SDE_OS_ENUM_CASE(ImageChannels::kRGB)
    SDE_OS_ENUM_CASE(ImageChannels::kRGBA)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const ImageShape& shape)
{
  return os << "{ height: " << shape.value.y() << ", width: " << shape.value.x() << " }";
}

std::ostream& operator<<(std::ostream& os, ImageError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(ImageError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(ImageError::kInvalidHandle)
    SDE_OS_ENUM_CASE(ImageError::kAssetNotFound)
    SDE_OS_ENUM_CASE(ImageError::kAssetInvalid)
    SDE_OS_ENUM_CASE(ImageError::kImageNotFound)
    SDE_OS_ENUM_CASE(ImageError::kUnsupportedBitDepth)
  }
  return os;
}

void ImageDataBufferDeleter::operator()(void* data) const { stbi_image_free(data); }

expected<void, ImageError> ImageCache::reload([[maybe_unused]] dependencies deps, Image& image)
{
  // Already loaded
  if (image.data_buffer.isValid())
  {
    return {};
  }

  // Check if image point is valid
  if (!asset::exists(image.path))
  {
    SDE_LOG_ERROR() << "AssetNotFound: " << SDE_OSNV(image.path);
    return make_unexpected(ImageError::kAssetNotFound);
  }

  // Set flag determining whether image should be flipped on load
  stbi_set_flip_vertically_on_load(image.options.flip_vertically);

  // Get STBI channel code
  const int channel_count_forced = to_stbi_enum(image.options.channels);

  // Load image data and sizing
  int height_on_load = 0;
  int width_on_load = 0;
  int channel_count_on_load = 0;
  void* image_data_ptr = nullptr;
  switch (image.options.element_type)
  {
  case TypeCode::kUInt8: {
    image_data_ptr = reinterpret_cast<void*>(stbi_load(
      image.path.string().c_str(), &height_on_load, &width_on_load, &channel_count_on_load, channel_count_forced));
    break;
  }
  case TypeCode::kUInt16: {
    image_data_ptr = reinterpret_cast<void*>(stbi_load_16(
      image.path.string().c_str(), &height_on_load, &width_on_load, &channel_count_on_load, channel_count_forced));
    break;
  }
  default: {
    SDE_LOG_ERROR() << "UnsupportedBitDepth: " << SDE_OSNV(image.options.element_type);
    return make_unexpected(ImageError::kUnsupportedBitDepth);
  }
  }

  // Check if image point is valid
  if (image_data_ptr == nullptr)
  {
    SDE_LOG_ERROR() << "AssetInvalid: " << SDE_OSNV(image.path);
    return make_unexpected(ImageError::kAssetInvalid);
  }

  SDE_LOG_DEBUG() << "Loaded image: " << SDE_OSNV(image.path) << ", " << SDE_OSNV(height_on_load) << ", "
                  << SDE_OSNV(width_on_load);

  // Set loaded image image
  image.options.channels = from_channel_count(channel_count_on_load);
  image.shape.value.x() = height_on_load;
  image.shape.value.y() = width_on_load;
  image.data_buffer = ImageDataBuffer{image_data_ptr};
  return {};
}

expected<void, ImageError> ImageCache::unload([[maybe_unused]] dependencies deps, Image& image)
{
  image.data_buffer = ImageDataBuffer{nullptr};
  image.shape.value.setZero();
  return {};
}

expected<Image, ImageError>
ImageCache::generate([[maybe_unused]] dependencies deps, const asset::path& image_path, const ImageOptions& options)
{
  Image info{
    .path = image_path, .options = options, .shape = {.value = {0, 0}}, .data_buffer = ImageDataBuffer{nullptr}};
  if (auto ok_or_error = reload(deps, info); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return info;
}

}  // namespace sde::graphics
