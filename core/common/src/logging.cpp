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
  return os << info.severity << ":" << info.file << ':' << info.line;
}

Log::~Log()
{
  if (this->isValid())
  {
    (*os_) << std::endl;
  }
}

Log::Log(std::ostream* target) : os_{target} {}

Log::Log() : Log{std::addressof(std::cout)} {}

Log::Log(Log&& other) { this->swap(other); }

Log& Log::operator=(Log&& other)
{
  this->swap(other);
  return *this;
}

void Log::swap(Log& other) { std::swap(this->os_, other.os_); }


Abort::~Abort()
{
  if (this->isValid())
  {
    (*this) << "\n\n********** RUNTIME ASSERTION FAILED **********\n";
    std::abort();
  }
}

Abort::Abort(std::ostream* target) : Log{target}
{
  if (this->isValid())
  {
    (*this) << "********** RUNTIME ASSERTION FAILED **********\n\n";
  }
}

Abort::Abort() : Abort{std::addressof(std::cerr)} {}

Abort::Abort(Abort&& other) { this->swap(other); }

}  // namespace sde
