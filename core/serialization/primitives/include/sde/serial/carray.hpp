/**
 * @copyright 2023-present Brian Cairl
 *
 * @file carray.hpp
 */
#pragma once

// C++ Standard Library
#include <iterator>

// SDE
#include "sde/serial/named.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename OArchiveT, typename ValueT, std::size_t N> struct save<OArchiveT, ValueT[N]>
{
  void operator()(OArchiveT& oar, const ValueT* array)
  {
    if constexpr (is_trivially_serializable_v<OArchiveT, ValueT>)
    {
      oar << named{"data", make_packet_fixed_size<N>(array)};
    }
    else
    {
      oar << named{"data", make_sequence(array, array + N)};
    }
  }
};

template <typename IArchiveT, typename ValueT, std::size_t N> struct load<IArchiveT, ValueT[N]>
{
  void operator()(IArchiveT& iar, ValueT* array)
  {
    if constexpr (is_trivially_serializable_v<IArchiveT, ValueT>)
    {
      auto p = make_packet_fixed_size<N>(array);
      iar >> named{"data", p};
    }
    else
    {
      auto s = make_sequence(array, array + N);
      iar >> named{"data", s};
    }
  }
};

}  // namespace sde::serial
