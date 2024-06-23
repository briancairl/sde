/**
 * @copyright 2024-present Brian Cairl
 *
 * @file asset.hpp
 */
#pragma once

// C++ Standard Library
#include <chrono>
#include <iosfwd>

// SDE
#include "sde/time.hpp"

namespace std
{

template <typename RepT, typename RatioT>
std::ostream& operator<<(std::ostream& os, const std::chrono::duration<RepT, RatioT>& period)
{
  return os << ::sde::toSeconds(period) << " s";
}

}  // namespace std

namespace sde
{

template <typename DurationT> std::ostream& operator<<(std::ostream& os, const BasicRate<DurationT>& rate)
{
  return os << toHertz(rate) << " hz";
}

}  // namespace sde