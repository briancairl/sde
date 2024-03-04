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
  case ImageChannels::kDefault:
    return os << "Default";
  case ImageChannels::kGrey:
    return os << "Grey";
  case ImageChannels::kGreyA:
    return os << "GreyA";
  case ImageChannels::kRGB:
    return os << "RGB";
  case ImageChannels::kRGBA:
    return os << "RGBA";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, ImageLoadFlags flags)
{
  return os << std::boolalpha << "{ flip_vertically: " << static_cast<bool>(flags.flip_vertically) << " }";
}

std::ostream& operator<<(std::ostream& os, const ImageShape& shape)
{
  return os << "{ height: " << shape.height << ", width: " << shape.width << " }";
}

std::ostream& operator<<(std::ostream& os, ImageLoadError error)
{
  switch (error)
  {
  case ImageLoadError::kResourceNotFound:
    return os << "ResourceNotFound";
  case ImageLoadError::kResourceInvalid:
    return os << "ResourceInvalid";
  case ImageLoadError::kUnsupportedBitDepth:
    return os << "UnsupportedBitDepth";
  }
  return os;
}

Image::Image(const ImageShape& shape, ImageChannels channels, TypeCode bit_depth, void* data) :
    shape_{shape}, channels_{channels}, bit_depth_{bit_depth}, data_{data}
{}

Image::Image(Image&& other) :
    shape_{other.shape_}, channels_{other.channels_}, bit_depth_{other.bit_depth_}, data_{other.data_}
{
  other.data_ = nullptr;
}

Image::~Image()
{
  if (data_ == nullptr)
  {
    return;
  }
  stbi_image_free(data_);
}

expected<Image, ImageLoadError> Image::load(const resource::path& image_path, const ImageOptions& options)
{
  // Check if image point is valid
  if (!resource::exists(image_path))
  {
    return make_unexpected(ImageLoadError::kResourceNotFound);
  }

  // Set flag determining whether image should be flipped on load
  stbi_set_flip_vertically_on_load(options.flags.flip_vertically);

  // Get STBI channel code
  const int channel_count_forced = to_stbi_enum(options.channels);

  // Load image data and sizing
  int height_on_load = 0;
  int width_on_load = 0;
  int channel_count_on_load = 0;
  void* image_data_ptr = nullptr;
  switch (options.bit_depth)
  {
  case TypeCode::kUInt8: {
    image_data_ptr = reinterpret_cast<void*>(stbi_load(
      image_path.string().c_str(), &height_on_load, &width_on_load, &channel_count_on_load, channel_count_forced));
    break;
  }
  case TypeCode::kUInt16: {
    image_data_ptr = reinterpret_cast<void*>(stbi_load_16(
      image_path.string().c_str(), &height_on_load, &width_on_load, &channel_count_on_load, channel_count_forced));
    break;
  }
  default: {
    return make_unexpected(ImageLoadError::kUnsupportedBitDepth);
  }
  }

  // Check if image point is valid
  if (image_data_ptr == nullptr)
  {
    return make_unexpected(ImageLoadError::kResourceInvalid);
  }

  return Image{
    {
      .height = static_cast<std::size_t>(height_on_load),
      .width = static_cast<std::size_t>(width_on_load),
    },
    ((options.channels == ImageChannels::kDefault) ? from_channel_count(channel_count_on_load) : options.channels),
    options.bit_depth,
    image_data_ptr};
}


std::ostream& operator<<(std::ostream& os, const Image& image)
{
  return os << "{ shape: " << image.shape() << ", depth: " << image.depth() << " }";
}

}  // namespace sde::graphics
