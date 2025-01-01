/**
 * @copyright 2022-present Brian Cairl
 *
 * @file hash_archive.hpp
 */
#pragma once

// C++ Standard Library
#include <string_view>
#include <type_traits>
#include <utility>

// SDE
#include "sde/hash.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/oarchive.hpp"
#include "sde/serial/ostream.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

class hash_archive : public oarchive<hash_archive>
{
  using oarchive_base = oarchive<hash_archive>;
  friend oarchive_base;

public:
  using oarchive_base::operator<<;
  using oarchive_base::operator&;

  explicit hash_archive(Hash initial = {}) : hash_{initial} {}

  hash_archive(hash_archive&& other) = default;
  hash_archive& operator=(hash_archive&& other) = default;

  const Hash& digest() const { return hash_; }

private:
  hash_archive(const hash_archive& other) = delete;
  hash_archive& operator=(const hash_archive& other) = delete;

  template <typename ValueT> constexpr expected<void, oarchive_error> write_impl(named<ValueT> named_value)
  {
    hash_ += ComputeHash(std::string_view{named_value.name});
    hash_ += ComputeTypeHash<ValueT>();
    return {};
  }

  template <typename IteratorT> expected<void, oarchive_error> write_impl([[maybe_unused]] const sequence<IteratorT>& _)
  {
    hash_ += ComputeTypeHash<IteratorT>();
    return {};
  }

  template <typename PointerT>
  expected<void, oarchive_error> write_impl([[maybe_unused]] const basic_packet<PointerT>& _)
  {
    hash_ += ComputeTypeHash<PointerT>();
    return {};
  }

  template <typename PointerT, std::size_t Len>
  expected<void, oarchive_error> write_impl([[maybe_unused]] const basic_packet_fixed_size<PointerT, Len>& _)
  {
    hash_ += ComputeTypeHash<PointerT>();
    hash_ += ComputeHash(Len);
    return {};
  }

  Hash hash_;
};

struct save_trivial_hash_archive
{
  template <typename ValueT> void operator()(hash_archive& ar, const ValueT& value)
  {
    ar << make_packet(std::addressof(value));
  }
};

template <typename ValueT>
struct save_impl<hash_archive, ValueT>
    : std::conditional_t<
        (is_trivially_serializable_v<hash_archive, ValueT> and !save_is_implemented_v<hash_archive, ValueT>),
        save_trivial_hash_archive,
        save<hash_archive, ValueT>>
{};


}  // namespace sde::serial
