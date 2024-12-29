/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_header.hpp
 */
#pragma once

// C++ Standard Library
#include <optional>
#include <string_view>

// SDE
#include "sde/game/native_script_typedefs.hpp"
#include "sde/serialization.hpp"
#include "sde/time.hpp"

namespace sde::game
{

struct native_script_header
{
  std::optional<TimeOffset> initialization_time_point = {};
  std::string_view name = {};
  script_id_t uid = 0;
  script_version_t version = 0;
};

}  // namespace sde::game

namespace sde::serial
{
template <typename Archive> struct serialize<Archive, game::native_script_header>
{
  void operator()(Archive& ar, game::native_script_header& data) const
  {
    ar& named{"uid", data.uid};
    ar& named{"version", data.version};
  }
};
}  // namespace sde::serial
