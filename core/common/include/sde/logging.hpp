/**
 * @copyright 2024-present Brian Cairl
 *
 * @file logging.hpp
 */
#ifdef SDE_COMMON_LOG
#error("Detected double include of logging.hpp")
#else
#define SDE_COMMON_LOG

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/format.hpp"

namespace sde
{

enum class LogSeverity
{
  kDebug,
  kInfo,
  kWarn,
  kError,
  kFatal
};

std::ostream& operator<<(std::ostream& os, LogSeverity severity);

struct LogFileInfo
{
  /// Log severity
  LogSeverity severity;
  /// File where log originate
  const char* file;
  /// File line where log originate
  int line;
};

std::ostream& operator<<(std::ostream& os, const LogFileInfo& info);

class Log
{
public:
  ~Log();

  Log();
  explicit Log(std::ostream* target);

  Log(Log&& other);
  Log& operator=(Log&& other);

  void swap(Log& other);
  void flush();

  constexpr bool isValid() const { return os_ != nullptr; }
  constexpr operator bool() const { return isValid(); }

  template <typename T> Log& operator<<(T&& obj)
  {
    (*os_) << std::forward<T>(obj);
    return *this;
  }

private:
  Log(const Log& other) = delete;
  Log& operator=(const Log& other) = delete;

  std::ostream* os_;
};

class Abort : public Log
{
public:
  ~Abort();

  Abort();
  explicit Abort(std::ostream* target);

  Abort(Abort&& other);
  Abort& operator=(Abort&& other);

private:
  Abort(const Abort& other) = delete;
  Abort& operator=(const Abort& other) = delete;
};


}  // namespace sde

#define __SDE_STR_EXPR(x) #x

#ifdef SDE_LOGGING_DISABLED

#define SDE_LOG_FMT(severity, fmt, ...) (void)0
#define SDE_LOG(severity, msg) (void)0

#else

#define SDE_LOG_GENERATE_FILE_INFO(severity)                                                                           \
  ::sde::LogFileInfo { severity, __FILE__, __LINE__ }

// C++ Standard Library
#include <cstdlib>
#include <iosfwd>

#define SDE_LOG(severity) sde::Log{} << "[SDE LOG] (" << SDE_LOG_GENERATE_FILE_INFO(severity) << ") "
#define SDE_LOG_FMT(severity, fmt, ...) SDE_LOG(severity) << sde::format<1024UL>(fmt, __VA_ARGS__)

#endif  // SDE_LOGGING_DISABLED

#define SDE_LOG_DEBUG_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kDebug, fmt, __VA_ARGS__)
#define SDE_LOG_INFO_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kInfo, fmt, __VA_ARGS__)
#define SDE_LOG_WARN_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kWarn, fmt, __VA_ARGS__)
#define SDE_LOG_ERROR_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kError, fmt, __VA_ARGS__)
#define SDE_LOG_FATAL_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kFatal, fmt, __VA_ARGS__)

#if defined(NDEBUG) || defined(_NDEBUG)
#define SDE_LOG_DEBUG()                                                                                                \
  if constexpr (false)                                                                                                 \
    sde::Log {}
#else
#define SDE_LOG_DEBUG() sde::Log{} << SDE_LOG_GENERATE_FILE_INFO(::sde::LogSeverity::kDebug) << ' '
#endif

#define SDE_LOG_INFO() SDE_LOG(::sde::LogSeverity::kInfo)
#define SDE_LOG_WARN() SDE_LOG(::sde::LogSeverity::kWarn)
#define SDE_LOG_ERROR() SDE_LOG(::sde::LogSeverity::kError)
#define SDE_LOG_FATAL() SDE_LOG(::sde::LogSeverity::kError)

#define SDE_FAIL() sde::Abort{} << SDE_LOG_GENERATE_FILE_INFO(::sde::LogSeverity::kFatal) << "\n\n"
#define SDE_ASSERT(condition)                                                                                          \
  if (!condition)                                                                                                      \
  SDE_FAIL() << "cond: " << __SDE_STR_EXPR(condition) << "\nexpl: "


#define SDE_ASSERT_OK(expected) SDE_ASSERT(expected.has_value()) << expected.error() << "\n\n      "
#define SDE_ASSERT_NULL(val_ptr) SDE_ASSERT(val_ptr == nullptr)
#define SDE_ASSERT_NON_NULL(val_ptr) SDE_ASSERT(val_ptr != nullptr)
#define SDE_ASSERT_TRUE(val_bool) SDE_ASSERT(static_cast<bool>(val_bool))
#define SDE_ASSERT_FALSE(val_bool) SDE_ASSERT(!static_cast<bool>(val_bool))
#define SDE_ASSERT_EQ(val_lhs, val_rhs) SDE_ASSERT((val_lhs == val_rhs))
#define SDE_ASSERT_NE(val_lhs, val_rhs) SDE_ASSERT((val_lhs != val_rhs))
#define SDE_ASSERT_LT(val_lhs, val_rhs) SDE_ASSERT((val_lhs < val_rhs))
#define SDE_ASSERT_LE(val_lhs, val_rhs) SDE_ASSERT((val_lhs <= val_rhs))
#define SDE_ASSERT_GT(val_lhs, val_rhs) SDE_ASSERT((val_lhs > val_rhs))
#define SDE_ASSERT_GE(val_lhs, val_rhs) SDE_ASSERT((val_lhs >= val_rhs))

#define SDE_UNREACHABLE() __builtin_unreachable()

#define SDE_ASSERT_NO_THROW(section)                                                                                   \
  try                                                                                                                  \
  {                                                                                                                    \
    section;                                                                                                           \
  }                                                                                                                    \
  catch (...)                                                                                                          \
  {                                                                                                                    \
    SDE_FAIL() << __SDE_STR_EXPR((section));                                                                           \
    SDE_UNREACHABLE();                                                                                                 \
  }

#define SDE_ASSERT_NO_EXCEPT(section, exception)                                                                       \
  try                                                                                                                  \
  {                                                                                                                    \
    section;                                                                                                           \
  }                                                                                                                    \
  catch (const exception& ex)                                                                                          \
  {                                                                                                                    \
    SDE_FAIL() << __SDE_STR_EXPR((section)) << "\n\nexception: " << ex.what();                                         \
    SDE_UNREACHABLE();                                                                                                 \
  }


#define SDE_SHOULD_NEVER_HAPPEN(reason)                                                                                \
  SDE_FAIL() << reason;                                                                                                \
  SDE_UNREACHABLE();

#define SDE_OS_ENUM_CASE(e)                                                                                            \
  case e: {                                                                                                            \
    return os << __SDE_STR_EXPR(e);                                                                                    \
  }
#define SDE_OSNV(x) '[' << __SDE_STR_EXPR(x) << "=" << x << ']'

#endif  // SDE_COMMON_LOG
