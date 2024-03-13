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
#include "sde/graphics/image_fwd.hpp"
#include "sde/graphics/typecode.hpp"

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
  texture_id_t id;
};

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
};

std::ostream& operator<<(std::ostream& os, const TextureInfo& info);

class TextureCache
{
public:
  using Textures = std::unordered_map<TextureHandle, TextureInfo, TextureHandleHash>;

  TextureCache() = default;
  ~TextureCache();

  void remove(const TextureHandle& index);

  TextureHandle create(const Image& image, const TextureOptions& options = {});

  TextureHandle
  create(std::uint8_t* const data, const TextureShape& shape, TextureLayout layout, const TextureOptions& options = {});

  TextureHandle create(
    std::uint16_t* const data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  TextureHandle create(
    std::uint32_t* const data,
    const TextureShape& shape,
    TextureLayout layout,
    const TextureOptions& options = {});

  const Textures& get() const { return textures_; }

private:
  Textures textures_;
};

}  // namespace sde::graphics

namespace std
{

template <> struct hash<sde::graphics::TextureHandle>
{
  constexpr std::size_t operator()(const sde::graphics::TextureHandle& handle) const { return handle.id; }
};

}  // namespace std