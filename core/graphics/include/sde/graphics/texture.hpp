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
#include "sde/graphics/image_fwd.hpp"
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

struct TextureHandle
{
  std::size_t id;
};

inline bool operator<(TextureHandle lhs, TextureHandle rhs) { return lhs.id < rhs.id; }
inline bool operator>(TextureHandle lhs, TextureHandle rhs) { return lhs.id > rhs.id; }
inline bool operator==(TextureHandle lhs, TextureHandle rhs) { return lhs.id == rhs.id; }
inline bool operator!=(TextureHandle lhs, TextureHandle rhs) { return lhs.id != rhs.id; }

struct TextureHandleHash
{
  constexpr std::size_t operator()(const TextureHandle& handle) const { return handle.id; }
};

std::ostream& operator<<(std::ostream& os, TextureHandle handle);

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

std::ostream& operator<<(std::ostream& os, const TextureOptions& error);

struct TextureShape
{
  std::size_t height;
  std::size_t width;
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
  kBackendMipMapGenerationFailure
};

std::ostream& operator<<(std::ostream& os, TextureError error);

class TextureCache
{
public:
  TextureCache() = default;
  ~TextureCache();

  bool remove(const TextureHandle& index);

  expected<void, TextureError> create(TextureHandle, const Image& image, const TextureOptions& options = {});

  expected<void, TextureError> create(
    TextureHandle texture,
    ContinuousView<const std::uint8_t> data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  expected<void, TextureError> create(
    TextureHandle texture,
    ContinuousView<const std::uint16_t> data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  expected<void, TextureError> create(
    TextureHandle texture,
    ContinuousView<const std::uint32_t> data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  expected<void, TextureError> create(
    TextureHandle texture,
    ContinuousView<const float> data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  expected<TextureHandle, TextureError> create(const Image& image, const TextureOptions& options = {})
  {
    auto texture = new_texture_handle();
    if (auto ok_or_error = create(texture, image, options); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
    return texture;
  }

  template <typename DataT>
  expected<TextureHandle, TextureError> create(
    ContinuousView<const DataT> data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {})
  {
    auto texture = new_texture_handle();
    if (auto ok_or_error = create(texture, data, shape, options); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
    return texture;
  }

  const TextureInfo* get(TextureHandle texture) const;

private:
  using TextureCacheMap = std::unordered_map<TextureHandle, TextureInfo, TextureHandleHash>;

  TextureHandle last_texture_handle_ = {1UL};

  TextureCacheMap textures_;

  TextureHandle new_texture_handle() { return TextureHandle{last_texture_handle_.id + 1UL}; }
};

}  // namespace sde::graphics

namespace std
{

template <> struct hash<sde::graphics::TextureHandle>
{
  constexpr std::size_t operator()(const sde::graphics::TextureHandle& handle) const { return handle.id; }
};

}  // namespace std