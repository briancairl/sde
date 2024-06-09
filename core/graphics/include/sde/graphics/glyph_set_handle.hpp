/**
 * @copyright 2024-present Brian Cairl
 *
 * @file type_set_handle.hpp
 */
#pragma once

// SDE
#include "sde/resource_handle.hpp"

namespace sde::graphics
{

struct TypeSetHandle : ResourceHandle<TypeSetHandle>
{
  explicit TypeSetHandle(id_type id) : ResourceHandle<TypeSetHandle>{id} {}
};

}  // namespace sde::graphics
