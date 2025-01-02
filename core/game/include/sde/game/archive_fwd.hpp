/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive_fwd.hpp
 */
#pragma once

#include "sde/serial/archive_fwd.hpp"
#include "sde/serialization_binary_file_fwd.hpp"

namespace sde::game
{

using VArchive = serial::hash_archive;
using OArchive = serial::associative_oarchive<serial::binary_ofarchive>;
using IArchive = serial::associative_iarchive<serial::binary_ifarchive>;

}  // namespace sde::game
