/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive.hpp
 */
#pragma once

#include "sde/serialization_binary_file.hpp"

namespace sde::game
{

using OArchive = serial::binary_ofarchive;
using IArchive = serial::binary_ifarchive;

}  // namespace sde::game
