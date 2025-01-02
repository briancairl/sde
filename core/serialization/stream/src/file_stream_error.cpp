// C++ Standard Library
#include <ostream>

// SDE
#include "sde/logging.hpp"
#include "sde/serial/file_stream_error.hpp"

namespace sde::serial
{

std::ostream& operator<<(std::ostream& os, file_stream_error error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(file_stream_error::kFileDoesNotExist)
    SDE_OS_ENUM_CASE(file_stream_error::kFileOpenFailed)
  }
  return os;
}

}  // namespace sde::serial