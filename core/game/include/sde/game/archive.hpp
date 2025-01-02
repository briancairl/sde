/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive.hpp
 */
#pragma once

#include "sde/serial/associative_archive.hpp"
#include "sde/serial/hash_archive.hpp"
#include "sde/serialization_binary_file.hpp"

namespace sde::game
{

using VArchive = serial::hash_archive;
using OArchive = serial::associative_oarchive<serial::binary_ofarchive>;
using IArchive = serial::associative_iarchive<serial::binary_ifarchive>;

}  // namespace sde::game
