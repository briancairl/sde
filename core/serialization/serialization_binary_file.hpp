/**
 * @copyright 2024-present Brian Cairl
 *
 * @file serialization_binary_file.hpp
 */
#pragma once

// This is a meta include for object serialization

// SDE
#include "sde/serial/binary_iarchive.hpp"
#include "sde/serial/binary_oarchive.hpp"
#include "sde/serial/file_istream.hpp"
#include "sde/serial/file_ostream.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

using binary_ofarchive = binary_oarchive<file_handle_ostream>;
using binary_ifarchive = binary_iarchive<file_handle_istream>;

}  // namespace sde::serial