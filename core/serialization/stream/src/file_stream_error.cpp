// C++ Standard Library
#include <ostream>

// SDE
#include "sde/logging.hpp"
#include "sde/serial/file_stream_error.hpp"

namespace sde::serial
{

std::ostream& operator<<(std::ostream& os, FileStreamError error)
{
  switch (error)
  {
    SDE_OSTREAM_ENUM_CASE(FileStreamError::kFileDoesNotExist)
    SDE_OSTREAM_ENUM_CASE(FileStreamError::kFileOpenFailed)
  }
  return os;
}

}  // namespace sde::serial