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
using OArchive = serial::binary_ofarchive;
using IArchive = serial::binary_ifarchive;
using OArchiveAssociative = serial::associative_oarchive<OArchive>;
using IArchiveAssociative = serial::associative_iarchive<IArchive>;

}  // namespace sde::game
