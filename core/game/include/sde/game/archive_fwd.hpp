/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive_fwd.hpp
 */
#pragma once

#include "sde/serialization_binary_file_fwd.hpp"

namespace sde::serial
{
class hash_oarchive;
}  // namespace sde::serial;

namespace sde::game
{

using VArchive = serial::hash_oarchive;
using OArchive = serial::binary_ofarchive;
using IArchive = serial::binary_ifarchive;

}  // namespace sde::game
