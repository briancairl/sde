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
#include "sde/expected.hpp"
#include "sde/serial/carray.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

enum class iarchive_error
{
  kReadFailure,
  kKeyMissing,
  kStreamError
};

template <typename OArchiveT> struct iarchive_traits
{
  using stream_type = void;
};

template <typename IArchiveT, typename ValueT> struct load_impl;

template <typename IArchiveT> class iarchive : public crtp_base<iarchive<IArchiveT>>
{
  template <typename ValueT>
  static constexpr bool is_primitive = is_named_v<ValueT> or is_packet_v<ValueT> or is_sequence_v<ValueT>;

public:
  template <typename ValueT>
  std::enable_if_t<!is_primitive<std::remove_const_t<std::remove_reference_t<ValueT>>>, expected<void, iarchive_error>>
  operator>>(ValueT&& value)
  {
    using CleanT = std::remove_const_t<std::remove_reference_t<ValueT>>;
    using LoadT = load_impl<IArchiveT, CleanT>;
    if constexpr (std::is_void_v<decltype(std::declval<LoadT&>()(this->derived(), std::forward<ValueT>(value)))>)
    {
      LoadT{}(this->derived(), std::forward<ValueT>(value));
      return {};
    }
    else
    {
      return LoadT{}(this->derived(), std::forward<ValueT>(value));
    }
  }

  template <typename IteratorT> expected<void, iarchive_error> operator>>(sequence<IteratorT> sequence)
  {
    return this->derived().read_impl(sequence);
  }

  template <typename ValueT> expected<void, iarchive_error> operator>>(named<ValueT> named_value)
  {
    return this->derived().read_impl(named_value);
  }

  template <typename PointerT> expected<void, iarchive_error> operator>>(basic_packet<PointerT> packet)
  {
    return this->derived().read_impl(packet);
  }

  template <typename PointerT, std::size_t Len>
  expected<void, iarchive_error> operator>>(basic_packet_fixed_size<PointerT, Len> packet)
  {
    return this->derived().read_impl(packet);
  }

  template <typename ValueT> expected<void, iarchive_error> operator&(ValueT&& value)
  {
    return this->operator>>(std::forward<ValueT>(value));
  }

  constexpr decltype(auto) stream() { return this->derived().stream_impl(); }
  constexpr decltype(auto) stream() const { return this->derived().stream_impl(); }

  iarchive() = default;

private:
  iarchive(const iarchive&) = default;
};

}  // namespace sde::serial
