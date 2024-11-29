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
  LogSeverity severity;
  const char* file;
  int line;
};

std::ostream& operator<<(std::ostream& os, const LogFileInfo& info);

class Abort
{
public:
  ~Abort();

  Abort();
  explicit Abort(std::ostream* target);

  Abort(Abort&& other);
  Abort& operator=(Abort&& other);

  void swap(Abort& other);

  template <typename T> Abort& operator<<(T&& obj)
  {
    if (os_ != nullptr)
    {
      (*os_) << std::forward<T>(obj);
    }
    return *this;
  }

private:
  Abort(const Abort& other) = delete;
  Abort& operator=(const Abort& other) = delete;

  std::ostream* os_;
};

}  // namespace sde

#ifdef SDE_LOGGING_DISABLED

#define SDE_LOG_FMT(severity, fmt, ...) (void)0
#define SDE_LOG(severity, msg) (void)0

#else

#define SDE_LOG_GENERATE_FILE_INFO(severity)                                                                           \
  ::sde::LogFileInfo { severity, __FILE__, __LINE__ }

// C++ Standard Library
#include <cstdio>
#include <cstdlib>
#include <iosfwd>

#define SDE_LOG_FMT(severity, fmt, ...)                                                                                \
  std::fprintf(stderr, sde::format("[SDE LOG] (%%s:%%d) %s\n", fmt), __FILE__, __LINE__, __VA_ARGS__)

#define SDE_LOG(severity, msg) SDE_LOG_FMT(severity, "%s", msg)

#endif  // SDE_LOGGING_DISABLED

#if defined(NDEBUG) || defined(_NDEBUG)
#define SDE_LOG_DEBUG_FMT(fmt, ...) (void)0
#else
#define SDE_LOG_DEBUG_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kDebug, fmt, __VA_ARGS__)
#endif

#define SDE_LOG_INFO_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kInfo, fmt, __VA_ARGS__)
#define SDE_LOG_WARN_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kWarn, fmt, __VA_ARGS__)
#define SDE_LOG_ERROR_FMT(fmt, ...) SDE_LOG_FMT(::sde::LogSeverity::kError, fmt, __VA_ARGS__)
#define SDE_LOG_FATAL_FMT(fmt, ...)                                                                                    \
  SDE_LOG_FMT(::sde::LogSeverity::kError, fmt, __VA_ARGS__);                                                           \
  {                                                                                                                    \
    std::abort();                                                                                                      \
  }

#if defined(NDEBUG) || defined(_NDEBUG)
#define SDE_LOG_DEBUG(msg) (void)0
#else
#define SDE_LOG_DEBUG(msg) SDE_LOG(::sde::LogSeverity::kDebug, msg)
#endif

#define SDE_STR_EXPR(x) #x

#define SDE_LOG_INFO(msg) SDE_LOG(::sde::LogSeverity::kInfo, msg)
#define SDE_LOG_WARN(msg) SDE_LOG(::sde::LogSeverity::kWarn, msg)
#define SDE_LOG_ERROR(msg) SDE_LOG(::sde::LogSeverity::kError, msg)
#define SDE_LOG_FATAL(msg) SDE_LOG(::sde::LogSeverity::kError, msg);

#define SDE_FAIL() sde::Abort{} << SDE_LOG_GENERATE_FILE_INFO(::sde::LogSeverity::kFatal) << "\n\n"
#define SDE_ASSERT(condition)                                                                                          \
  if (!condition)                                                                                                      \
  SDE_FAIL() << "cond: " << SDE_STR_EXPR(condition) << "\nexpl: "


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
#define SDE_SHOULD_NEVER_HAPPEN(reason)                                                                                \
  SDE_FAIL() << reason;                                                                                                \
  SDE_UNREACHABLE();


#endif  // SDE_COMMON_LOG
