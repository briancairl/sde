/**
 * @copyright 2024-present Brian Cairl
 *
 * @file rendered_2D.hpp
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
  texture_id_t id;
};

struct Texture2DHandleHash
{
  constexpr std::size_t operator()(const Texture2DHandle& handle) const { return handle.id; }
};

std::ostream& operator<<(std::ostream& os, Texture2DHandle handle);

/*
 * @brief Image loading options
 */
struct Texture2DFlags
{
  std::uint8_t unpack_alignment : 1;
};

std::ostream& operator<<(std::ostream& os, Texture2DFlags flags);

enum class Texture2DWrapping : std::uint8_t
{
  kClampToBorder,
  kRepeat
};

std::ostream& operator<<(std::ostream& os, Texture2DWrapping wrapping);

enum class Texture2DSampling : std::uint8_t
{
  kLinear,
  kNearest
};

std::ostream& operator<<(std::ostream& os, Texture2DSampling sampling);

/**
 * @brief Texture creation options
 */
struct Texture2DOptions
{
  Texture2DWrapping u_wrapping = Texture2DWrapping::kClampToBorder;
  Texture2DWrapping v_wrapping = Texture2DWrapping::kClampToBorder;

  Texture2DSampling min_sampling = Texture2DSampling::kNearest;
  Texture2DSampling mag_sampling = Texture2DSampling::kNearest;

  Texture2DFlags flags = {0};
};

std::ostream& operator<<(std::ostream& os, const Texture2DOptions& error);

struct Texture2DShape
{
  std::size_t height;
  std::size_t width;
};

std::ostream& operator<<(std::ostream& os, const Texture2DShape& shape);

struct Texture2DInfo
{
  Texture2DLayout layout;
  Texture2DShape shape;
};

std::ostream& operator<<(std::ostream& os, const Texture2DInfo& info);

class Texture2DCache
{
public:
  using Textures = std::unordered_map<Texture2DHandle, Texture2DInfo, Texture2DHandleHash>;

  Texture2DCache() = default;
  ~Texture2DCache();

  void remove(const Texture2DHandle& index);

  Texture2DHandle create(const Image& image, const Texture2DOptions& options = {});

  Texture2DHandle create(
    std::uint8_t* const data,
    const Texture2DShape& shape,
    Texture2DLayout layout,
    const Texture2DOptions& options = {});

  Texture2DHandle create(
    std::uint16_t* const data,
    const Texture2DShape& shape,
    Texture2DLayout layout,
    const Texture2DOptions& options = {});

  Texture2DHandle create(
    std::uint32_t* const data,
    const Texture2DShape& shape,
    Texture2DLayout layout,
    const Texture2DOptions& options = {});

  const Textures& get() const { return textures_; }

private:
  Textures textures_;
};

}  // namespace sde::graphics

namespace std
{

template <> struct hash<sde::graphics::Texture2DHandle>
{
  constexpr std::size_t operator()(const sde::graphics::Texture2DHandle& handle) const { return handle.id; }
};

}  // namespace std