/**
 * @copyright 2024-present Brian Cairl
 *
 * @file image_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct ImageHandle : ResourceHandle<ImageHandle>
{
  ImageHandle() = default;
  explicit ImageHandle(id_type id) : ResourceHandle<ImageHandle>{id} {}
};

}  // namespace sde::graphics
