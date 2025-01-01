/**
 * @copyright 2022-present Brian Cairl
 *
 * @file binary_oarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

// SDE
#include "sde/serial/named.hpp"
#include "sde/serial/oarchive.hpp"
#include "sde/serial/ostream.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename OStreamT> class binary_oarchive;

template <typename OStreamT> struct oarchive_traits<binary_oarchive<OStreamT>>
{
  using stream_type = OStreamT;
};

template <typename OStreamT> class binary_oarchive : public oarchive<binary_oarchive<OStreamT>>
{
  using oarchive_base = oarchive<binary_oarchive<OStreamT>>;
  friend oarchive_base;

public:
  explicit binary_oarchive(ostream<OStreamT>& os) : os_{static_cast<OStreamT*>(std::addressof(os))} {}

  binary_oarchive(binary_oarchive&& other) { this->swap(other); }

  binary_oarchive& operator=(binary_oarchive&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(binary_oarchive& other) { std::swap(this->os_, other.os_); }

  using oarchive_base::operator<<;
  using oarchive_base::operator&;

private:
  binary_oarchive(const binary_oarchive& other) = delete;
  binary_oarchive& operator=(const binary_oarchive& other) = delete;

  constexpr OStreamT* stream_impl() { return os_; }
  constexpr const OStreamT* stream_impl() const { return os_; }

  template <typename ValueT> constexpr expected<void, oarchive_error> write_impl(const named<ValueT>& named_value)
  {
    (*this) << named_value.value;
    return {};
  }

  template <typename IteratorT> constexpr expected<void, oarchive_error> write_impl(const sequence<IteratorT>& sequence)
  {
    const auto [first, last] = sequence;
    for (auto itr = first; itr != last; ++itr)
    {
      (*this) << (*itr);
    }
    return {};
  }

  template <typename PointerT> constexpr expected<void, oarchive_error> write_impl(const basic_packet<PointerT>& packet)
  {
    using value_type = std::remove_pointer_t<PointerT>;
    if constexpr (std::is_void_v<value_type>)
    {
      os_->write(packet.data, packet.len);
    }
    else
    {
      os_->write(packet.data, packet.len * sizeof(value_type));
    }
    return {};
  }

  template <typename PointerT, std::size_t Len>
  constexpr expected<void, oarchive_error> write_impl(const basic_packet_fixed_size<PointerT, Len>& packet)
  {
    using value_type = std::remove_pointer_t<PointerT>;
    if constexpr (std::is_void_v<value_type>)
    {
      os_->write(packet.data, packet.len);
    }
    else
    {
      os_->write(packet.data, packet.len * sizeof(value_type));
    }
    return {};
  }

  OStreamT* os_ = nullptr;
};

template <typename OStreamT> binary_oarchive(ostream<OStreamT>& os) -> binary_oarchive<OStreamT>;

struct save_trivial_binary_oarchive
{
  template <typename OStreamT, typename ValueT> void operator()(binary_oarchive<OStreamT>& ar, const ValueT& value)
  {
    ar << make_packet(std::addressof(value));
  }
};

template <typename OStreamT, typename ValueT>
struct save_impl<binary_oarchive<OStreamT>, ValueT>
    : std::conditional_t<
        (is_trivially_serializable_v<binary_oarchive<OStreamT>, ValueT> and
         !save_is_implemented_v<binary_oarchive<OStreamT>, ValueT>),
        save_trivial_binary_oarchive,
        save<binary_oarchive<OStreamT>, ValueT>>
{};

}  // namespace sde::serial
