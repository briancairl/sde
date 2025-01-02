/**
 * @copyright 2024-present Brian Cairl
 *
 * @file file_stream_error.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

namespace sde::serial
{

enum class file_stream_error
{
  kFileDoesNotExist,
  kFileOpenFailed,
};

std::ostream& operator<<(std::ostream& os, file_stream_error error);

}  // namespace sde::serial
