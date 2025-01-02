/**
 * @copyright 2022-present Brian Cairl
 *
 * @file iarchive.hpp
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

enum class iarchive_error
{
  kReadFailure,
  kLoadFailure,
  kKeyMissing,
  kStreamError
};

inline std::ostream& operator<<(std::ostream& os, iarchive_error error)
{
  switch (error)
  {
  case iarchive_error::kReadFailure:
    return os << "iarchive_error::kReadFailure";
  case iarchive_error::kLoadFailure:
    return os << "iarchive_error::kLoadFailure";
  case iarchive_error::kKeyMissing:
    return os << "iarchive_error::kKeyMissing";
  case iarchive_error::kStreamError:
    return os << "iarchive_error::kStreamError";
  }
  return os;
}

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
  template <typename ValueT> expected<void, iarchive_error> operator>>(ValueT&& value)
  {
    static_assert(!std::is_const_v<ValueT>);
    using LoadT = std::remove_reference_t<ValueT>;
    if constexpr (is_primitive<LoadT>)
    {
      return this->derived().read_impl(std::forward<ValueT>(value));
    }
    else
    {
      using LoadImplT = load_impl<IArchiveT, LoadT>;
      if constexpr (std::is_void_v<decltype(std::declval<LoadImplT&>()(this->derived(), std::forward<ValueT>(value)))>)
      {
        LoadImplT{}(this->derived(), std::forward<ValueT>(value));
        return {};
      }
      else
      {
        return LoadImplT{}(this->derived(), std::forward<ValueT>(value));
      }
    }
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
