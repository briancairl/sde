/**
 * @copyright 2022-present Brian Cairl
 *
 * @file iarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

// SDE
#include "sde/crtp.hpp"
#include "sde/serial/carray.hpp"
#include "sde/serial/label.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename IArchiveT, typename ValueT> struct load_impl;

template <typename IArchiveT> class iarchive : public crtp_base<iarchive<IArchiveT>>
{
  template <typename ValueT>
  static constexpr bool is_primitive = is_label_v<ValueT> or is_packet_v<ValueT> or is_sequence_v<ValueT>;

public:
  template <typename ValueT> IArchiveT& operator&(ValueT&& value)
  {
    using CleanT = std::remove_const_t<std::remove_reference_t<ValueT>>;
    load_impl<IArchiveT, CleanT>{}(this->derived(), std::forward<ValueT>(value));
    return this->derived();
  }

  template <typename ValueT> std::enable_if_t<!is_primitive<ValueT>, IArchiveT&> operator>>(ValueT&& value)
  {
    using CleanT = std::remove_const_t<std::remove_reference_t<ValueT>>;
    load_impl<IArchiveT, CleanT>{}(this->derived(), std::forward<ValueT>(value));
    return this->derived();
  }

  template <typename IteratorT> IArchiveT& operator>>(sequence<IteratorT> sequence)
  {
    this->derived().read_impl(sequence);
    return this->derived();
  }

  template <typename ValueT> IArchiveT& operator>>(label<ValueT> l)
  {
    this->derived().read_impl(l);
    return this->derived();
  }

  template <typename PointerT> IArchiveT& operator>>(basic_packet<PointerT> packet)
  {
    this->derived().read_impl(packet);
    return this->derived();
  }

  template <typename PointerT, std::size_t Len> IArchiveT& operator>>(basic_packet_fixed_size<PointerT, Len> packet)
  {
    this->derived().read_impl(packet);
    return this->derived();
  }

  iarchive() = default;

private:
  iarchive(const iarchive&) = default;

  template <typename ValueT> static constexpr void read_impl(label<ValueT> _) {}
};

}  // namespace sde::serial
