/**
 * @copyright 2022-present Brian Cairl
 *
 * @file hash_oarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <utility>

// SDE
#include "sde/hash.hpp"
#include "sde/serial/oarchive.hpp"
#include "sde/serial/ostream.hpp"
#include "sde/serial/packet.hpp"

namespace sde::serial
{

class hash_oarchive : public oarchive<hash_oarchive>
{
  using oarchive_base = oarchive<hash_oarchive>;

  friend oarchive_base;

public:
  using oarchive_base::operator<<;
  using oarchive_base::operator&;

  const Hash& digest() const { return hash_; }

private:
  template <typename ValueT> void write_impl(const label<ValueT> l)
  {
    hash_ += ComputeHash(l.value);
    hash_ += ComputeTypeHash<ValueT>();
  }

  template <typename IteratorT> void write_impl([[maybe_unused]] const sequence<IteratorT>& _)
  {
    hash_ += ComputeTypeHash<IteratorT>();
  }

  template <typename PointerT> void write_impl([[maybe_unused]] const basic_packet<PointerT>& _)
  {
    hash_ += ComputeTypeHash<PointerT>();
  }

  template <typename PointerT, std::size_t Len>
  void write_impl([[maybe_unused]] const basic_packet_fixed_size<PointerT, Len>& _)
  {
    hash_ += ComputeTypeHash<PointerT>();
    hash_ += ComputeHash(Len);
  }

  Hash hash_;
};

struct save_trivial_hash_oarchive
{
  template <typename ValueT> void operator()(hash_oarchive& ar, const ValueT& value)
  {
    ar << make_packet(std::addressof(value));
  }
};

template <typename ValueT>
struct save_impl<hash_oarchive, ValueT>
    : std::conditional_t<
        (is_trivially_serializable_v<hash_oarchive, ValueT> and !save_is_implemented_v<hash_oarchive, ValueT>),
        save_trivial_hash_oarchive,
        save<hash_oarchive, ValueT>>
{};


}  // namespace sde::serial
