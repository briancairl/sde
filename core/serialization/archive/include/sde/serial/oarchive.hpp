/**
 * @copyright 2022-present Brian Cairl
 *
 * @file oarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <type_traits>
#include <utility>

// SDE
#include "sde/crtp.hpp"
#include "sde/expected.hpp"
#include "sde/serial/carray.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

enum class oarchive_error
{
  kWriteFailure,
  kSaveFailure,
  kKeyRepeated,
  kStreamError
};

inline std::ostream& operator<<(std::ostream& os, oarchive_error error)
{
  switch (error)
  {
  case oarchive_error::kWriteFailure:
    return os << "oarchive_error::kWriteFailure";
  case oarchive_error::kSaveFailure:
    return os << "oarchive_error::kSaveFailure";
  case oarchive_error::kKeyRepeated:
    return os << "oarchive_error::kKeyRepeated";
  case oarchive_error::kStreamError:
    return os << "oarchive_error::kStreamError";
  }
  return os;
}

template <typename OArchiveT> struct oarchive_traits
{
  using stream_type = void;
};

template <typename OArchiveT, typename ValueT> struct save_impl;

template <typename OArchiveT> class oarchive : public crtp_base<oarchive<OArchiveT>>
{
  template <typename ValueT>
  static constexpr bool is_primitive = is_named_v<ValueT> or is_packet_v<ValueT> or is_sequence_v<ValueT>;

public:
  template <typename ValueT> expected<void, oarchive_error> operator<<(const ValueT& value)
  {
    using SaveT = std::remove_const_t<std::remove_reference_t<ValueT>>;
    if constexpr (is_primitive<SaveT>)
    {
      return this->derived().write_impl(value);
    }
    else
    {
      using SaveImplT = save_impl<OArchiveT, SaveT>;
      if constexpr (std::is_void_v<decltype(std::declval<SaveImplT&>()(this->derived(), value))>)
      {
        SaveImplT{}(this->derived(), value);
        return {};
      }
      else
      {
        return SaveImplT{}(this->derived(), value);
      }
    }
  }

  template <typename ValueT> expected<void, oarchive_error> operator&(ValueT&& value)
  {
    return this->operator<<(std::forward<ValueT>(value));
  }

  constexpr decltype(auto) stream() { return this->derived().stream_impl(); }
  constexpr decltype(auto) stream() const { return this->derived().stream_impl(); }

  oarchive() = default;

private:
  oarchive(const oarchive&) = default;
};

}  // namespace sde::serial
