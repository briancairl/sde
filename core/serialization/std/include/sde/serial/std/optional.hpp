/**
 * @copyright 2023-present Brian Cairl
 *
 * @file optional.hpp
 */
#pragma once

// C++ Standard Library
#include <optional>

// SDE
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/std/string.hpp"

namespace sde::serial
{

template <typename OArchiveT, typename T> struct save<OArchiveT, std::optional<T>>
{
  void operator()(OArchiveT& oar, const std::optional<T>& opt)
  {
    if constexpr (is_trivially_serializable_v<OArchiveT, T>)
    {
      oar << named{"data", make_packet(std::addressof(opt))};
    }
    else
    {
      oar << named{"has_value", opt.has_value()};
      if (opt.has_value())
      {
        oar << named{"value", *opt};
      }
    }
  }
};

template <typename IArchiveT, typename T> struct load<IArchiveT, std::optional<T>>
{
  void operator()(IArchiveT& iar, std::optional<T>& opt)
  {
    if constexpr (is_trivially_serializable_v<IArchiveT, T>)
    {
      auto p = make_packet(std::addressof(opt));
      iar >> named{"data", p};
    }
    else
    {
      bool has_value{false};
      iar >> named{"has_value", has_value};
      if (has_value)
      {
        iar >> named{"value", opt.emplace()};
      }
    }
  }
};

}  // namespace sde::serial
