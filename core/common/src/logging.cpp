// C++ Standard Library
#include <cstdlib>
#include <iostream>

// SDE
#include "sde/logging.hpp"

namespace sde
{
namespace
{

std::ostream* __log_stream_defaults[static_cast<std::size_t>(LogSeverity::_kN)] = {
  std::addressof(std::cout),
  std::addressof(std::cout),
  std::addressof(std::cout),
  std::addressof(std::cerr),
  std::addressof(std::cerr)};

std::ostream* __log_streams[static_cast<std::size_t>(LogSeverity::_kN)] = {
  std::addressof(std::cout),
  std::addressof(std::cout),
  std::addressof(std::cout),
  std::addressof(std::cerr),
  std::addressof(std::cerr)};

}  // namespace

void setLogStream(LogSeverity severity, std::ostream* os)
{
  SDE_ASSERT_NE(severity, LogSeverity::_kN);
  const auto i = static_cast<std::size_t>(severity);
  if (os == nullptr)
  {
    __log_streams[i] = __log_stream_defaults[i];
  }
  else
  {
    __log_streams[i] = os;
  }
}

void setLogStream(std::ostream* os)
{
  if (os == nullptr)
  {
    for (std::size_t i = 0; i < static_cast<std::size_t>(LogSeverity::_kN); ++i)
    {
      __log_streams[i] = __log_stream_defaults[i];
    }
  }
  else
  {
    for (std::size_t i = 0; i < static_cast<std::size_t>(LogSeverity::_kN); ++i)
    {
      __log_streams[i] = os;
    }
  }
}

std::ostream* getLogStream(LogSeverity severity)
{
  SDE_ASSERT_NE(severity, LogSeverity::_kN);
  const auto i = static_cast<std::size_t>(severity);
  return __log_streams[i];
}

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
  case LogSeverity::_kN:
    SDE_FAIL();
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

Log::Log(Log&& other) { this->swap(other); }

Log& Log::operator=(Log&& other)
{
  this->swap(other);
  return *this;
}

void Log::flush() { this->os_->flush(); }

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
    this->flush();
    (*this) << "********** RUNTIME ASSERTION FAILED **********\n\n";
  }
}

Abort::Abort() : Abort{std::addressof(std::cerr)} {}

Abort::Abort(Abort&& other) : Abort{nullptr} { this->swap(other); }

}  // namespace sde
