/**
 * @copyright 2023-present Brian Cairl
 *
 * @file tuple.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>
#include <utility>

// SDE
#include "sde/format.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"

namespace sde::serial
{
template <typename OArchiveT, typename... Ts> struct save<OArchiveT, std::tuple<Ts...>>
{
  void operator()(OArchiveT& oar, const std::tuple<Ts...>& tup)
  {
    if constexpr ((is_trivially_serializable_v<OArchiveT, Ts> && ...))
    {
      oar << named{"data", make_packet(&tup)};
    }
    else
    {
      expand(oar, tup, std::make_integer_sequence<std::size_t, sizeof...(Ts)>{});
    }
  }

private:
  template <typename Tup, std::size_t... Is>
  static int expand(OArchiveT& oar, Tup&& tup, std::integer_sequence<std::size_t, Is...> _)
  {
    return ((oar << named{sde::format("%lu", Is), std::get<Is>(std::forward<Tup>(tup))}, 0) + ...);
  }
};

template <typename IArchiveT, typename... Ts> struct load<IArchiveT, std::tuple<Ts...>>
{
  void operator()(IArchiveT& iar, std::tuple<Ts...>& tup)
  {
    if constexpr ((is_trivially_serializable_v<IArchiveT, Ts> && ...))
    {
      auto p = make_packet(&tup);
      iar >> named{"data", p};
    }
    else
    {
      expand(iar, tup, std::make_integer_sequence<std::size_t, sizeof...(Ts)>{});
    }
  }

private:
  template <typename Tup, std::size_t... Is>
  static int expand(IArchiveT& iar, Tup&& tup, std::integer_sequence<std::size_t, Is...> _)
  {
    return ((iar >> named{sde::format("%lu", Is), std::get<Is>(std::forward<Tup>(tup))}, 0) + ...);
  }
};

}  // namespace sde::serial
