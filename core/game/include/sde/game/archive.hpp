/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive.hpp
 */
#pragma once

#include "sde/serial/hash_oarchive.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::game
{

using VArchive = serial::hash_oarchive;
using OArchive = serial::binary_ofarchive;
using IArchive = serial::binary_ifarchive;

}  // namespace sde::game
