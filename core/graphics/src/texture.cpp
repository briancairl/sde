// C++ Standard Library
#include <algorithm>
#include <iomanip>
#include <iterator>
#include <ostream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/geometry_utils.hpp"
#include "sde/graphics/image.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/logging.hpp"
#include "sde/resource_wrapper.hpp"

namespace sde::graphics
{
namespace
{
enum_t to_native_layout_enum(const TextureLayout channels)
{
  switch (channels)
  {
  case TextureLayout::kR:
    return GL_RED;
  case TextureLayout::kRG:
    return GL_RG;
  case TextureLayout::kRGB:
    return GL_RGB;
  case TextureLayout::kRGBA:
    return GL_RGBA;
  }
  return GL_RED;
}

enum_t to_native_wrapping_mode_enum(const TextureWrapping mode)
{
  switch (mode)
  {
  case TextureWrapping::kClampToBorder:
    return GL_CLAMP_TO_BORDER;
  case TextureWrapping::kClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case TextureWrapping::kRepeat:
    return GL_REPEAT;
  default:
    break;
  }
  return GL_CLAMP_TO_BORDER;
}

enum_t to_native_sampling_mode_enum(const TextureSampling mode)
{
  switch (mode)
  {
  case TextureSampling::kLinear:
    return GL_LINEAR;
  case TextureSampling::kNearest:
    return GL_NEAREST;
  default:
    break;
  }
  return GL_NEAREST;
}

TextureNativeID allocate_texture_2D_and_bind(
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  TextureNativeID texture_id{[] {
    native_texture_id_t id;
    glGenTextures(1, &id);
    return id;
  }()};

  glBindTexture(GL_TEXTURE_2D, texture_id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, to_native_wrapping_mode_enum(options.u_wrapping));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, to_native_wrapping_mode_enum(options.v_wrapping));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, to_native_sampling_mode_enum(options.min_sampling));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, to_native_sampling_mode_enum(options.mag_sampling));

  if (options.flags.unpack_alignment)
  {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  }

  static constexpr GLint kDefaultLevelOfDetail = 0;
  static constexpr GLint kDefaultBorder = 0;
  glTexImage2D(
    GL_TEXTURE_2D,
    kDefaultLevelOfDetail,
    to_native_layout_enum(layout),
    shape.value.x(),
    shape.value.y(),
    kDefaultBorder,
    to_native_layout_enum(layout),
    to_native_typecode(type),
    nullptr);

  return texture_id;
}

expected<void, TextureError> upload_texture_2D(
  const void* const data,
  const TextureLayout layout,
  const TypeCode type,
  const Vec2i& offset,
  const Vec2i& shape)
{
  static constexpr GLint kDefaultLevelOfDetail = 0;

  glTexSubImage2D(
    GL_TEXTURE_2D,
    kDefaultLevelOfDetail,
    offset.x(),
    offset.y(),
    shape.x(),
    shape.y(),
    to_native_layout_enum(layout),
    to_native_typecode(type),
    reinterpret_cast<const void*>(data));

  if (has_active_error())
  {
    SDE_LOG_DEBUG_FMT(
      "BackendTransferFailure: [offset_x=%d, offset_y=%d, shape_x=%d, shape_y=%d, format=%s, type=%s]",
      offset.x(),
      offset.y(),
      shape.x(),
      shape.y(),
      static_cast<int>(to_native_layout_enum(layout)),
      static_cast<int>(to_native_typecode(type)));
    return make_unexpected(TextureError::kBackendTransferFailure);
  }

  return {};
}

expected<TextureNativeID, TextureError> create_native_texture_2D(
  const void* const data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  auto texture_id = allocate_texture_2D_and_bind(shape, layout, options, type);

  if (has_active_error())
  {
    SDE_LOG_DEBUG("BackendCreationFailure");
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  if (const auto ok_or_error = upload_texture_2D(data, layout, type, Vec2i::Zero(), shape.value);
      !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  if (options.flags.generate_mip_map)
  {
    glGenerateMipmap(GL_TEXTURE_2D);
    if (has_active_error())
    {
      SDE_LOG_DEBUG("BackendMipMapGenerationFailure");
      return make_unexpected(TextureError::kBackendMipMapGenerationFailure);
    }
  }

  return texture_id;
}

expected<TextureNativeID, TextureError> create_native_texture_2D(
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options,
  const TypeCode type)
{
  auto texture_id = allocate_texture_2D_and_bind(shape, layout, options, type);

  if (has_active_error())
  {
    SDE_LOG_DEBUG("BackendCreationFailure");
    return make_unexpected(TextureError::kBackendCreationFailure);
  }

  return texture_id;
}

std::size_t to_channel_count(const TextureLayout channels)
{
  switch (channels)
  {
  case TextureLayout::kR:
    return 1;
  case TextureLayout::kRG:
    return 2;
  case TextureLayout::kRGB:
    return 3;
  case TextureLayout::kRGBA:
    return 4;
  }
  return 0;
}

TextureLayout layout_from_channel_count(std::size_t channel_count)
{
  switch (channel_count)
  {
  case 1:
    return TextureLayout::kR;
  case 2:
    return TextureLayout::kRG;
  case 3:
    return TextureLayout::kRGB;
  case 4:
    return TextureLayout::kRGBA;
  }
  return TextureLayout::kR;
}

std::size_t size_in_bytes(const Vec2i& extents, TextureLayout layout)
{
  return (static_cast<std::size_t>(extents.prod()) * to_channel_count(layout));
}

template <typename DataT>
expected<TextureInfo, TextureError>
create_texture_impl(View<DataT> data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options)
{
  if (!data)
  {
    SDE_LOG_DEBUG("InvalidDataValue");
    return make_unexpected(TextureError::kInvalidDataValue);
  }
  else if (shape.height() == 0 or shape.width() == 0)
  {
    SDE_LOG_DEBUG("InvalidDimensions");
    return make_unexpected(TextureError::kInvalidDimensions);
  }

  const std::size_t required_size = size_in_bytes(shape.value, layout);
  const std::size_t actual_size = sizeof(DataT) * data.size();
  if (actual_size != required_size)
  {
    SDE_LOG_DEBUG_FMT("Expected texture to have data len %lu but has %lu", required_size, actual_size);
    return make_unexpected(TextureError::kInvalidDataLength);
  }

  auto texture_or_error =
    create_native_texture_2D(reinterpret_cast<const void*>(data.data()), shape, layout, options, typecode<DataT>());
  if (!texture_or_error.has_value())
  {
    return make_unexpected(texture_or_error.error());
  }

  return TextureInfo{
    .layout = layout, .shape = shape, .options = options, .native_id = std::move(texture_or_error).value()};
}

template <typename DataT>
expected<TextureInfo, TextureError>
create_texture_impl(const TextureShape& shape, TextureLayout layout, const TextureOptions& options)
{
  auto texture_or_error = create_native_texture_2D(shape, layout, options, typecode<DataT>());

  if (!texture_or_error.has_value())
  {
    return make_unexpected(texture_or_error.error());
  }

  return TextureInfo{
    .layout = layout, .shape = shape, .options = options, .native_id = std::move(texture_or_error).value()};
}

}  // namespace

void TextureNativeDeleter::operator()(native_texture_id_t id) const
{
  if (id != 0)
  {
    SDE_LOG_DEBUG_FMT("glDeleteTextures(1, &%u)", id);
    glDeleteTextures(1, &id);
  }
}

std::ostream& operator<<(std::ostream& os, const TextureShape& shape)
{
  return os << "{ height: " << shape.value.y() << ", width: " << shape.value.x() << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureInfo& info)
{
  return os << "{ shape: " << info.shape << ", layout: " << info.layout << ", options: " << info.options
            << ", native_id: " << info.native_id << " }";
}

std::ostream& operator<<(std::ostream& os, TextureWrapping wrapping)
{
  switch (wrapping)
  {
  case TextureWrapping::kClampToBorder:
    return os << "ClampToBorder";
  case TextureWrapping::kClampToEdge:
    return os << "ClampToEdge";
  case TextureWrapping::kRepeat:
    return os << "Repeat";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureSampling sampling)
{
  switch (sampling)
  {
  case TextureSampling::kLinear:
    return os << "Linear";
  case TextureSampling::kNearest:
    return os << "Nearest";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureLayout layout)
{
  switch (layout)
  {
  case TextureLayout::kR:
    return os << "R";
  case TextureLayout::kRG:
    return os << "RG";
  case TextureLayout::kRGB:
    return os << "RGB";
  case TextureLayout::kRGBA:
    return os << "RGBA";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, TextureError error)
{
  switch (error)
  {
  case TextureError::kAssetNotFound:
    return os << "AssetNotFound";
  case TextureError::kAssetLoadingFailed:
    return os << "AssetLoadingFailed";
  case TextureError::kElementAlreadyExists:
    return os << "ElementAlreadyExists";
  case TextureError::kInvalidHandle:
    return os << "InvalidHandle";
  case TextureError::kInvalidDimensions:
    return os << "InvalidDimensions";
  case TextureError::kInvalidDataValue:
    return os << "InvalidDataValue";
  case TextureError::kInvalidDataLength:
    return os << "InvalidDataLength";
  case TextureError::kBackendCreationFailure:
    return os << "BackendCreationFailure";
  case TextureError::kBackendTransferFailure:
    return os << "BackendTransferFailure";
  case TextureError::kBackendMipMapGenerationFailure:
    return os << "BackendMipMapGenerationFailure";
  case TextureError::kReplaceAreaEmpty:
    return os << "ReplaceAreaEmpty";
  case TextureError::kReplaceAreaOutOfBounds:
    return os << "ReplaceAreaOutOfBounds";
  }
}

std::ostream& operator<<(std::ostream& os, TextureFlags flags)
{
  return os << std::boolalpha << "{ unpack_alignment: " << static_cast<bool>(flags.unpack_alignment)
            << ", generate_mip_map: " << static_cast<bool>(flags.generate_mip_map) << " }";
}

std::ostream& operator<<(std::ostream& os, const TextureOptions& options)
{
  return os << "{ u_wrapping: " << options.u_wrapping << ", v_wrapping: " << options.v_wrapping
            << ", min_sampling: " << options.min_sampling << ", mag_sampling: " << options.mag_sampling
            << ", flags: " << options.flags << " }";
}

bool operator==(const TextureFlags& lhs, const TextureFlags& rhs)
{
  return std::memcmp(std::addressof(lhs), std::addressof(rhs), sizeof(TextureFlags)) == 0;
}

bool operator==(const TextureOptions& lhs, const TextureOptions& rhs)
{
  return std::memcmp(std::addressof(lhs), std::addressof(rhs), sizeof(TextureOptions)) == 0;
}

bool operator==(const TextureInfo& lhs, const TextureInfo& rhs)
{
  return std::memcmp(std::addressof(lhs), std::addressof(rhs), sizeof(TextureInfo)) == 0;
}

template <typename DataT>
expected<void, TextureError> replace(const TextureInfo& texture_info, View<const DataT> data, const Bounds2i& area)
{
  if (area.isEmpty())
  {
    SDE_LOG_DEBUG("ReplaceAreaEmpty");
    return make_unexpected(TextureError::kReplaceAreaEmpty);
  }

  const std::size_t required_size = size_in_bytes(area.max() - area.min(), texture_info.layout);
  const std::size_t actual_size = sizeof(DataT) * data.size();
  if (actual_size != required_size)
  {
    SDE_LOG_DEBUG("InvalidDataLength");
    return make_unexpected(TextureError::kInvalidDataLength);
  }

  if (Bounds2i{Vec2i::Zero(), texture_info.shape.value}.contains(area))
  {
    glBindTexture(GL_TEXTURE_2D, texture_info.native_id);
    return upload_texture_2D(data.data(), texture_info.layout, typecode<DataT>(), area.min(), area.max() - area.min());
  }

  SDE_LOG_DEBUG("ReplaceAreaOutOfBounds");
  return make_unexpected(TextureError::kReplaceAreaOutOfBounds);
}

template expected<void, TextureError>
replace(const TextureInfo& texture_info, View<const std::uint8_t> data, const Bounds2i& area);

template expected<void, TextureError>
replace(const TextureInfo& texture_info, View<const std::uint16_t> data, const Bounds2i& area);

template expected<void, TextureError>
replace(const TextureInfo& texture_info, View<const std::uint32_t> data, const Bounds2i& area);

template expected<void, TextureError>
replace(const TextureInfo& texture_info, View<const float> data, const Bounds2i& area);

template <typename DataT>
expected<TextureInfo, TextureError> TextureCache::generate(
  View<const DataT> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options)
{
  return create_texture_impl(data, shape, layout, options);
}

template expected<TextureInfo, TextureError> TextureCache::generate(
  View<const std::uint8_t> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<TextureInfo, TextureError> TextureCache::generate(
  View<const std::uint16_t> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<TextureInfo, TextureError> TextureCache::generate(
  View<const std::uint32_t> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<TextureInfo, TextureError> TextureCache::generate(
  View<const float> data,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);


expected<TextureInfo, TextureError> TextureCache::generate(const Image& image, const TextureOptions& options)
{
  return TextureCache::generate(
    image.data(), TextureShape{image.shape().value}, layout_from_channel_count(image.getChannelCount()), options);
}

template <typename DataT>
expected<TextureInfo, TextureError> TextureCache::generate(
  TypeTag<const DataT> /*_*/,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options)
{
  return create_texture_impl<DataT>(shape, layout, options);
}

template expected<TextureInfo, TextureError> TextureCache::generate(
  TypeTag<const std::uint8_t> /*_*/,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<TextureInfo, TextureError> TextureCache::generate(
  TypeTag<const std::uint16_t> /*_*/,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<TextureInfo, TextureError> TextureCache::generate(
  TypeTag<const std::uint32_t> /*_*/,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

template expected<TextureInfo, TextureError> TextureCache::generate(
  TypeTag<const float> /*_*/,
  const TextureShape& shape,
  const TextureLayout layout,
  const TextureOptions& options);

TextureCache::result_type TextureCacheLoader::operator()(
  TextureCache& cache,
  const asset::path& path,
  const ImageOptions& image_options,
  const TextureOptions& texture_options) const
{
  if (auto texture_source_image = Image::load(path, image_options); texture_source_image.has_value())
  {
    SDE_LOG_INFO_FMT("texture data loaded from disk: %s", path.string().c_str());
    return cache.create(*texture_source_image, texture_options);
  }
  SDE_LOG_DEBUG("AssetLoadingFailed");
  return make_unexpected(TextureError::kAssetLoadingFailed);
}

TextureCache::result_type TextureCacheLoader::operator()(
  TextureCache& cache,
  const asset::path& path,
  const TextureOptions& texture_options) const
{
  static constexpr ImageOptions kImageOptions{.flags = {.flip_vertically = true}};
  return this->operator()(cache, path, kImageOptions, texture_options);
}

}  // namespace sde::graphics
