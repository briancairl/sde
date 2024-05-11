/**
 * @copyright 2024-present Brian Cairl
 *
 * @file assert.hpp
 */
#ifdef SDE_COMMON_LOG
#error("Detected double include of assert.hpp")
#else
#define SDE_COMMON_LOG

namespace sde
{

enum class LogSeverity
{
  kDebug,
  kInfo,
  kWarn,
  kError
};

}  // namespace sde

#ifdef SDE_LOGGING_DISABLED

#define SDE_LOG(severity, text) (void)0

#else

// C++ Standard Library
#include <cstdio>
#include <cstdlib>

#define SDE_LOG(severity, text)                                                                                        \
  std::fprintf((severity == LogSeverity::kError) ? stderr : stdout, "[SDE LOG] (%s:%d) %s\n", __FILE__, __LINE__, text)

#endif  // SDE_LOGGING_DISABLED

#ifdef NDEBUG

#define SDE_ASSERT_MSG(fmt, ...) (void)0
#define SDE_ASSERT_MSG_COND(cond, fmt, ...) (void)0
#define SDE_ASSERT(cond, message)                                                                                      \
  if (!(cond))                                                                                                         \
  {                                                                                                                    \
    std::abort();                                                                                                      \
  }

#else

#define SDE_ASSERT_MSG(fmt, ...) std::fprintf(stderr, fmt, __VA_ARGS__)
#define SDE_ASSERT_MSG_COND(cond, fmt, ...)                                                                            \
  if (!(cond))                                                                                                         \
  {                                                                                                                    \
    SDE_ASSERT_MSG(fmt, __VA_ARGS__);                                                                                  \
  }
#define SDE_ASSERT(cond, message)                                                                                      \
  if (!(cond))                                                                                                         \
  {                                                                                                                    \
    SDE_ASSERT_MSG(                                                                                                    \
      "\n***RUNTIME ASSERTION FAILED***\n\ncondition : %s\nmessage   : %s\nfile      : %s:%d\\n",                      \
      #cond,                                                                                                           \
      message,                                                                                                         \
      __FILE__,                                                                                                        \
      __LINE__);                                                                                                       \
    std::abort();                                                                                                      \
  }

#endif  // NDEBUG


#define SDE_ASSERT_NULL_MSG(val_ptr, msg) SDE_ASSERT(val_ptr == nullptr, msg)
#define SDE_ASSERT_NON_NULL_MSG(val_ptr, msg) SDE_ASSERT(val_ptr != nullptr, msg)
#define SDE_ASSERT_TRUE_MSG(val_bool, msg) SDE_ASSERT(static_cast<bool>(val_bool), msg)
#define SDE_ASSERT_FALSE_MSG(val_bool, msg) SDE_ASSERT(!static_cast<bool>(val_bool), msg)
#define SDE_ASSERT_EQ_MSG(val_lhs, val_rhs, msg) SDE_ASSERT((val_lhs == val_rhs), msg)
#define SDE_ASSERT_NE_MSG(val_lhs, val_rhs, msg) SDE_ASSERT((val_lhs != val_rhs), msg)
#define SDE_ASSERT_LT_MSG(val_lhs, val_rhs, msg) SDE_ASSERT((val_lhs < val_rhs), msg)
#define SDE_ASSERT_LE_MSG(val_lhs, val_rhs, msg) SDE_ASSERT((val_lhs <= val_rhs), msg)
#define SDE_ASSERT_GT_MSG(val_lhs, val_rhs, msg) SDE_ASSERT((val_lhs > val_rhs), msg)
#define SDE_ASSERT_GE_MSG(val_lhs, val_rhs, msg) SDE_ASSERT((val_lhs >= val_rhs), msg)

#define SDE_ASSERT_NULL(val_ptr) SDE_ASSERT_NULL_MSG(val_ptr, "expected pointer to have NULL value")
#define SDE_ASSERT_NON_NULL(val_ptr) SDE_ASSERT_NON_NULL_MSG(val_ptr, "expected pointer to have non-NULL value")
#define SDE_ASSERT_TRUE(val_bool) SDE_ASSERT_TRUE_MSG(val_bool, "expected expression to evaluate to TRUE")
#define SDE_ASSERT_FALSE(val_bool) SDE_ASSERT_FALSE_MSG(val_bool, "expected expression to evaluate to FALSE")
#define SDE_ASSERT_EQ(val_lhs, val_rhs) SDE_ASSERT_EQ_MSG(val_lhs, val_rhs, "expected values to be equal")
#define SDE_ASSERT_NE(val_lhs, val_rhs) SDE_ASSERT_NE_MSG(val_lhs, val_rhs, "expected values to be unequal")
#define SDE_ASSERT_LT(val_lhs, val_rhs)                                                                                \
  SDE_ASSERT_LT_MSG(val_lhs, val_rhs, "expected left value to be less than right value")
#define SDE_ASSERT_LE(val_lhs, val_rhs)                                                                                \
  SDE_ASSERT_LE_MSG(val_lhs, val_rhs, "expected left value to be less or equal to than right value")
#define SDE_ASSERT_GT(val_lhs, val_rhs)                                                                                \
  SDE_ASSERT_GT_MSG(val_lhs, val_rhs, "expected left value to be greater than right value")
#define SDE_ASSERT_GE(val_lhs, val_rhs)                                                                                \
  SDE_ASSERT_GE_MSG(val_lhs, val_rhs, "expected left value to be greater than or equal to right value")

#endif  // SDE_COMMON_LOG
