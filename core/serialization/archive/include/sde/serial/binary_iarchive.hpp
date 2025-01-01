/**
 * @copyright 2022-present Brian Cairl
 *
 * @file binary_iarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

// SDE
#include "sde/serial/iarchive.hpp"
#include "sde/serial/istream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename IStreamT> class binary_iarchive;

template <typename IStreamT> struct iarchive_traits<binary_iarchive<IStreamT>>
{
  using stream_type = IStreamT;
};

template <typename IStreamT> class binary_iarchive : public iarchive<binary_iarchive<IStreamT>>
{
  using iarchive_base = iarchive<binary_iarchive<IStreamT>>;

  friend iarchive_base;

public:
  explicit binary_iarchive(istream<IStreamT>& is) : is_{static_cast<IStreamT*>(std::addressof(is))} {}

  binary_iarchive(binary_iarchive&& other) { this->swap(other); }

  binary_iarchive& operator=(binary_iarchive&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(binary_iarchive& other) { std::swap(this->is_, other.is_); }

  using iarchive_base::operator>>;
  using iarchive_base::operator&;

private:
  binary_iarchive(const binary_iarchive& other) = delete;
  binary_iarchive& operator=(const binary_iarchive& other) = delete;

  constexpr IStreamT* stream_impl() { return is_; }
  constexpr const IStreamT* stream_impl() const { return is_; }

  template <typename ValueT> constexpr expected<void, iarchive_error> read_impl(named<ValueT> named_value)
  {
    (*this) >> named_value.value;
    return {};
  }

  template <typename IteratorT> constexpr expected<void, iarchive_error> read_impl(sequence<IteratorT> sequence)
  {
    const auto [first, last] = sequence;
    for (auto itr = first; itr != last; ++itr)
    {
      (*this) >> (*itr);
    }
    return {};
  }

  template <typename PointerT> constexpr expected<void, iarchive_error> read_impl(basic_packet<PointerT> packet)
  {
    using value_type = std::remove_pointer_t<PointerT>;
    if constexpr (std::is_void_v<value_type>)
    {
      is_->read(packet.data, packet.len);
    }
    else
    {
      is_->read(packet.data, packet.len * sizeof(value_type));
    }
    return {};
  }

  template <typename PointerT, std::size_t Len>
  constexpr expected<void, iarchive_error> read_impl(basic_packet_fixed_size<PointerT, Len> packet)
  {
    using value_type = std::remove_pointer_t<PointerT>;
    if constexpr (std::is_void_v<value_type>)
    {
      is_->read(packet.data, packet.len);
    }
    else
    {
      is_->read(packet.data, packet.len * sizeof(value_type));
    }
    return {};
  }

  IStreamT* is_ = nullptr;
};

template <typename IStreamT> binary_iarchive(istream<IStreamT>& is) -> binary_iarchive<IStreamT>;

struct load_trivial
{
  template <typename IStreamT, typename ValueT> void operator()(binary_iarchive<IStreamT>& ar, ValueT& value)
  {
    ar >> make_packet(std::addressof(value));
  }
};

template <typename IStreamT, typename ValueT>
struct load_impl<binary_iarchive<IStreamT>, ValueT>
    : std::conditional_t<
        (is_trivially_serializable_v<binary_iarchive<IStreamT>, ValueT> and
         !load_is_implemented_v<binary_iarchive<IStreamT>, ValueT>),
        load_trivial,
        load<binary_iarchive<IStreamT>, ValueT>>
{};

}  // namespace sde::serial
