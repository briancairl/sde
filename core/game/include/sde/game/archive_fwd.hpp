/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive_fwd.hpp
 */
#pragma once

#include "sde/serialization_binary_file_fwd.hpp"

namespace sde::game
{

using OArchive = serial::binary_ofarchive;
using IArchive = serial::binary_ifarchive;

}  // namespace sde::game
