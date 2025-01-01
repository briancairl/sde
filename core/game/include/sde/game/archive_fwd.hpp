/**
 * @copyright 2024-present Brian Cairl
 *
 * @file archive_fwd.hpp
 */
#pragma once

#include "sde/serialization_binary_file_fwd.hpp"

namespace sde::serial
{
class hash_archive;
}  // namespace sde::serial;

namespace sde::game
{

using VArchive = serial::hash_archive;
using OArchive = serial::binary_ofarchive;
using IArchive = serial::binary_ifarchive;

}  // namespace sde::game
