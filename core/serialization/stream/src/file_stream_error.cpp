// C++ Standard Library
#include <ostream>

// SDE
#include "sde/serial/file_stream_error.hpp"

namespace sde::serial
{

std::ostream& operator<<(std::ostream& os, FileStreamError error)
{
  switch (error)
  {
  case FileStreamError::kFileDoesNotExist: {
    return os << "FileDoesNotExist";
  }
  case FileStreamError::kFileOpenFailed: {
    return os << "FileOpenFailed";
  }
  }
  return os;
}

}  // namespace sde::serial