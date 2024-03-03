/**
 * @copyright 2024-present Brian Cairl
 *
 * @file rendered_2D.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <vector>

// SDE
#include "sde/graphics/image_fwd.hpp"

namespace sde::graphics
{

/**
 * @brief Texture channel layout specifier
 */
enum class Texture2DLayout : std::uint8_t
{
  kR,
  kRG,
  kRGB,
  kRGBA
};

std::ostream& operator<<(std::ostream& os, Texture2DLayout layout);

struct Texture2DHandle
{
  std::uint32_t id;
};

std::ostream& operator<<(std::ostream& os, Texture2DHandle handle);

struct Texture2DShape
{
  std::size_t height;
  std::size_t width;
};

std::ostream& operator<<(std::ostream& os, const Texture2DShape& shape);

struct Texture2D
{
  Texture2DHandle handle;
  Texture2DLayout layout;
  Texture2DShape shape;
};

std::ostream& operator<<(std::ostream& os, const Texture2D& texture);

class Texture2DCache
{
public:
  Texture2DCache() = default;
  ~Texture2DCache();

  const void remove(const std::size_t index) { textures_.erase(index); }

  const Texture2D& create(const Image& image);

  const Texture2D& create(std::uint8_t* const data, const Texture2DShape& shape, Texture2DLayout layout);

  const Texture2D& create(std::uint16_t* const data, const Texture2DShape& shape, Texture2DLayout layout);

  const Texture2D& create(std::uint32_t* const data, const Texture2DShape& shape, Texture2DLayout layout);

  const Texture2D& operator[](const std::size_t index) const { return textures_[index]; }

  const std::size_t size() const { textures_.size(); }

private:
  std::vector<Texture2D> textures_;
};

} // namespace sde::graphics
