/**
 * @copyright 2024-present Brian Cairl
 *
 * @file serialization_binary_file_fwd.hpp
 */
#pragma once

// This is a meta include for object serialization

// SDE
#include "sde/serialization_fwd.hpp"

namespace sde::serial
{

using binary_ofarchive = binary_oarchive<file_handle_ostream>;
using binary_ifarchive = binary_iarchive<file_handle_istream>;

}  // namespace sde::serial