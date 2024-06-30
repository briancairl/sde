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

enum class FileStreamError
{
  kFileDoesNotExist,
  kFileOpenFailed,
};

std::ostream& operator<<(std::ostream& os, FileStreamError error);

}  // namespace sde::serial
