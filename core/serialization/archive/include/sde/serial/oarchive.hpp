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
  kKeyRepeated,
  kStreamError
};

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
  template <typename ValueT>
  std::enable_if_t<!is_primitive<std::remove_const_t<std::remove_reference_t<ValueT>>>, expected<void, oarchive_error>>
  operator<<(const ValueT& value)
  {
    using CleanT = std::remove_const_t<std::remove_reference_t<ValueT>>;
    using SaveT = save_impl<OArchiveT, CleanT>;
    if constexpr (std::is_void_v<decltype(std::declval<SaveT&>()(this->derived(), value))>)
    {
      SaveT{}(this->derived(), value);
      return {};
    }
    else
    {
      return SaveT{}(this->derived(), value);
    }
  }

  template <typename IteratorT> expected<void, oarchive_error> operator<<(const sequence<IteratorT>& sequence)
  {
    return this->derived().write_impl(sequence);
  }

  template <typename ValueT> expected<void, oarchive_error> operator<<(const named<ValueT>& named_value)
  {
    return this->derived().write_impl(named_value);
  }

  template <typename PointerT> expected<void, oarchive_error> operator<<(const basic_packet<PointerT>& packet)
  {
    return this->derived().write_impl(packet);
  }

  template <typename PointerT, std::size_t Len>
  expected<void, oarchive_error> operator<<(const basic_packet_fixed_size<PointerT, Len>& packet)
  {
    return this->derived().write_impl(packet);
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
