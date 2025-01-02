/**
 * @copyright 2023-present Brian Cairl
 *
 * @file utility.hpp
 */
#pragma once

// C++ Standard Library
#include <utility>

// SDE
#include "sde/serial/named.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"

namespace sde::serial
{

template <typename OArchiveT, typename FirstT, typename SecondT> struct save<OArchiveT, std::pair<FirstT, SecondT>>
{
  void operator()(OArchiveT& oar, const std::pair<FirstT, SecondT>& pair)
  {
    if constexpr (is_trivially_serializable_v<OArchiveT, FirstT> and is_trivially_serializable_v<OArchiveT, SecondT>)
    {
      oar << named{"data", make_packet(&pair)};
    }
    else
    {
      oar << named{"first", pair.first};
      oar << named{"second", pair.second};
    }
  }
};

template <typename IArchiveT, typename FirstT, typename SecondT> struct load<IArchiveT, std::pair<FirstT, SecondT>>
{
  void operator()(IArchiveT& iar, std::pair<FirstT, SecondT>& pair)
  {
    if constexpr (is_trivially_serializable_v<IArchiveT, FirstT> and is_trivially_serializable_v<IArchiveT, SecondT>)
    {
      auto p = make_packet(&pair);
      iar >> named{"data", p};
    }
    else
    {
      iar >> named{"first", pair.first};
      iar >> named{"second", pair.second};
    }
  }
};

}  // namespace sde::serial
