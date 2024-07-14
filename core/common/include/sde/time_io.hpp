/**
 * @copyright 2024-present Brian Cairl
 *
 * @file time_io.hpp
 */
#pragma once

// SDE
#include "sde/serialization.hpp"
#include "sde/time.hpp"

namespace sde::serial
{

template <typename ArchiveT, typename Rep, typename Period>
struct is_trivially_serializable<ArchiveT, std::chrono::duration<Rep, Period>> : std::true_type
{};

template <typename Archive, typename DurationT> struct save<Archive, BasicRate<DurationT>>
{
  void operator()(Archive& ar, const BasicRate<DurationT>& rate) const { ar << named{"period", rate.period()}; }
};

template <typename Archive, typename DurationT> struct load<Archive, BasicRate<DurationT>>
{
  void operator()(Archive& ar, BasicRate<DurationT>& rate) const
  {
    DurationT period;
    ar >> named{"period", period};
    rate = BasicRate{period};
  }
};

}  // namespace sde::serial
