/**
 * @copyright 2023-present Brian Cairl
 *
 * @file carray.hpp
 */
#pragma once

// C++ Standard Library
#include <tuple>

// SDE
#include "sde/serial/iarchive.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/oarchive.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename... NamedFieldTs> struct structure
{
  static_assert((sizeof...(NamedFieldTs) > 0UL), "'structure' must contain at least one field");
  static_assert((is_named_v<NamedFieldTs> and ...), "'structure' must contain uniquely named fields");

  structure(NamedFieldTs... fields) : fields{std::move(fields)...} {}

  std::tuple<NamedFieldTs...> fields;
};

template <typename OArchiveT, typename... NamedFieldTs> struct save<OArchiveT, structure<NamedFieldTs...>>
{
  expected<void, oarchive_error> operator()(OArchiveT& oar, const structure<NamedFieldTs...>& structure)
  {
    if (std::apply([&oar](auto... field) { return ((oar << field).has_value() and ...); }, structure.fields))
    {
      return {};
    }
    return make_unexpected(oarchive_error::kSaveFailure);
  }
};

template <typename IArchiveT, typename... NamedFieldTs> struct load<IArchiveT, structure<NamedFieldTs...>>
{
  expected<void, iarchive_error> operator()(IArchiveT& iar, structure<NamedFieldTs...> structure)
  {
    if (std::apply([&iar](auto... field) { return ((iar >> field).has_value() and ...); }, structure.fields))
    {
      return {};
    }
    return make_unexpected(iarchive_error::kLoadFailure);
  }
};

}  // namespace sde::serial
