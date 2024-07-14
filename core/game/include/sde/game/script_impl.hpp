/**
 * @copyright 2024-present Brian Cairl
 *
 * @file script_impl.hpp
 */
#pragma once

// EnTT
#include <entt/entt.hpp>

// Common
#include "sde/expected.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"

// Game
#include "sde/game/assets.hpp"
#include "sde/game/script_runtime.hpp"
#include "sde/game/systems.hpp"


namespace sde::serial
{

template <typename Archive> struct load<Archive, audio::SoundHandle> : load<Archive, ResourceHandle<audio::SoundHandle>>
{};
template <typename Archive> struct save<Archive, audio::SoundHandle> : save<Archive, ResourceHandle<audio::SoundHandle>>
{};

template <typename Archive>
struct load<Archive, graphics::FontHandle> : load<Archive, ResourceHandle<graphics::FontHandle>>
{};
template <typename Archive>
struct save<Archive, graphics::FontHandle> : save<Archive, ResourceHandle<graphics::FontHandle>>
{};

template <typename Archive>
struct load<Archive, graphics::TextureHandle> : load<Archive, ResourceHandle<graphics::TextureHandle>>
{};
template <typename Archive>
struct save<Archive, graphics::TextureHandle> : save<Archive, ResourceHandle<graphics::TextureHandle>>
{};

template <typename Archive>
struct load<Archive, graphics::ShaderHandle> : load<Archive, ResourceHandle<graphics::ShaderHandle>>
{};
template <typename Archive>
struct save<Archive, graphics::ShaderHandle> : save<Archive, ResourceHandle<graphics::ShaderHandle>>
{};

template <typename Archive>
struct load<Archive, graphics::TileSetHandle> : load<Archive, ResourceHandle<graphics::TileSetHandle>>
{};
template <typename Archive>
struct save<Archive, graphics::TileSetHandle> : save<Archive, ResourceHandle<graphics::TileSetHandle>>
{};

template <typename Archive>
struct load<Archive, graphics::TypeSetHandle> : load<Archive, ResourceHandle<graphics::TypeSetHandle>>
{};
template <typename Archive>
struct save<Archive, graphics::TypeSetHandle> : save<Archive, ResourceHandle<graphics::TypeSetHandle>>
{};

template <typename Archive>
struct load<Archive, graphics::RenderTargetHandle> : load<Archive, ResourceHandle<graphics::RenderTargetHandle>>
{};
template <typename Archive>
struct save<Archive, graphics::RenderTargetHandle> : save<Archive, ResourceHandle<graphics::RenderTargetHandle>>
{};

}  // namespace sde::serial