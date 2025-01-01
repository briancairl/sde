/**
 * @copyright 2022-present Brian Cairl
 *
 * @file oarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

// SDE
#include "sde/crtp.hpp"
#include "sde/serial/carray.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename OArchiveT, typename ValueT> struct save_impl;

template <typename OArchiveT> class oarchive : public crtp_base<oarchive<OArchiveT>>
{
  template <typename ValueT>
  static constexpr bool is_primitive = is_named_v<ValueT> or is_packet_v<ValueT> or is_sequence_v<ValueT>;

public:
  template <typename ValueT>
  std::enable_if_t<!is_primitive<std::remove_const_t<std::remove_reference_t<ValueT>>>, OArchiveT&>
  operator<<(const ValueT& value)
  {
    using CleanT = std::remove_const_t<std::remove_reference_t<ValueT>>;
    save_impl<OArchiveT, CleanT>{}(this->derived(), value);
    return this->derived();
  }

  template <typename IteratorT> OArchiveT& operator<<(const sequence<IteratorT>& sequence)
  {
    this->derived().write_impl(sequence);
    return this->derived();
  }

  template <typename ValueT> OArchiveT& operator<<(const named<ValueT>& named_value)
  {
    this->derived().write_impl(named_value);
    return this->derived();
  }

  template <typename PointerT> OArchiveT& operator<<(const basic_packet<PointerT>& packet)
  {
    this->derived().write_impl(packet);
    return this->derived();
  }

  template <typename PointerT, std::size_t Len>
  OArchiveT& operator<<(const basic_packet_fixed_size<PointerT, Len>& packet)
  {
    this->derived().write_impl(packet);
    return this->derived();
  }

  template <typename ValueT> OArchiveT& operator&(ValueT&& value)
  {
    return this->operator<<(std::forward<ValueT>(value));
  }

  oarchive() = default;

private:
  oarchive(const oarchive&) = default;
};

}  // namespace sde::serial
