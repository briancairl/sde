/**
 * @copyright 2024-present Brian Cairl
 *
 * @file asset.hpp
 */
#pragma once

// C++ Standard Library
#include <chrono>
#include <type_traits>

namespace sde
{

template <typename ScalarT = float, typename Rep, typename Period>
constexpr ScalarT toSeconds(const std::chrono::duration<Rep, Period>& duration)
{
  static_assert(std::is_floating_point_v<ScalarT>);
  using SecondsT = std::chrono::duration<ScalarT>;
  return std::chrono::duration_cast<SecondsT>(duration).count();
}

template <typename DurationT, typename ScalarT = float> constexpr DurationT toTimeOffset(const ScalarT seconds)
{
  static_assert(std::is_floating_point_v<ScalarT>);
  constexpr auto kNanosPerSecond = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds{1});
  using tick_type = typename std::chrono::nanoseconds::rep;
  const auto nano_ticks = static_cast<tick_type>(kNanosPerSecond.count() * seconds);
  return std::chrono::duration_cast<DurationT>(std::chrono::nanoseconds{nano_ticks});
}

template <typename DurationT> class BasicRate
{
public:
  constexpr explicit BasicRate(DurationT period) : period_{period} {}

  constexpr const DurationT& period() const { return period_; }

  constexpr operator const DurationT&() const { return period(); }

  template <typename HertzT> constexpr static auto fromHertz(HertzT hz)
  {
    static_assert(std::is_floating_point_v<HertzT>);
    return BasicRate{toTimeOffset<DurationT>(static_cast<HertzT>(1) / hz)};
  }

private:
  DurationT period_;
};

template <typename ScalarT = float, typename DurationT> constexpr ScalarT toHertz(const BasicRate<DurationT>& rate)
{
  static_assert(std::is_floating_point_v<ScalarT>);
  return static_cast<ScalarT>(1) / toSeconds<ScalarT>(rate.period());
}

template <typename DurationOrRepT, typename DurationT>
constexpr auto operator*(const DurationOrRepT& lhs, const BasicRate<DurationT>& rhs)
{
  return lhs / rhs.period();
}

template <typename DurationOrRepT, typename DurationT>
constexpr auto operator/(const DurationOrRepT& lhs, const BasicRate<DurationT>& rhs)
{
  return lhs * rhs.period();
}

template <typename DurationOrRepT, typename DurationT>
constexpr auto operator*(const BasicRate<DurationT>& lhs, const DurationOrRepT& rhs)
{
  return lhs.period() / rhs;
}

template <typename DurationOrRepT, typename DurationT>
constexpr auto operator/(const BasicRate<DurationT>& lhs, const DurationOrRepT& rhs)
{
  return lhs.period() * rhs;
}

using Clock = std::chrono::steady_clock;
using TimeOffset = Clock::duration;
using Rate = BasicRate<TimeOffset>;

using WallClock = std::chrono::system_clock;
using WallTimeOffset = WallClock::duration;
using WallRate = BasicRate<WallTimeOffset>;

template <typename TimeOffsetT = TimeOffset, typename ScalarT> constexpr auto Hertz(ScalarT hz)
{
  return BasicRate<TimeOffsetT>::fromHertz(hz);
}

}  // namespace sde