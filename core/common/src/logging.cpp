// C++ Standard Library
#include <cstdlib>
#include <iostream>

// SDE
#include "sde/logging.hpp"

namespace sde
{

std::ostream& operator<<(std::ostream& os, LogSeverity severity)
{
  switch (severity)
  {
  case LogSeverity::kDebug:
    return os << "Debug";
  case LogSeverity::kInfo:
    return os << "Info";
  case LogSeverity::kWarn:
    return os << "Warn";
  case LogSeverity::kError:
    return os << "Error";
  case LogSeverity::kFatal:
    return os << "Fatal";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const LogFileInfo& info)
{
  // clang-format off
  return os << "type: " << info.severity << '\n'
            << "file: " << info.file << ':'
                        << info.line << '\n';
  // clang-format on
}

Abort::~Abort()
{
  if (os_ == nullptr)
  {
    return;
  }

  {
    (*os_) << "\n\n********** RUNTIME ASSERTION FAILED **********\n";
    std::abort();
  }
}

Abort::Abort(std::ostream* target) : os_{target}
{
  if (os_ == nullptr)
  {
    return;
  }

  {
    (*os_) << "********** RUNTIME ASSERTION FAILED **********\n\n";
  }
}

Abort::Abort() : Abort{std::addressof(std::cerr)} {}

Abort::Abort(Abort&& other) { this->swap(other); }

Abort& Abort::operator=(Abort&& other)
{
  this->swap(other);
  return *this;
}

void Abort::swap(Abort& other) { std::swap(this->os_, other.os_); }

}  // namespace sde
