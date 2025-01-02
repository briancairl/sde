/**
 * @copyright 2025-present Brian Cairl
 *
 * @file associative_archive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <unordered_map>
#include <utility>

// SDE
#include "sde/format.hpp"
#include "sde/hash.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/oarchive.hpp"
#include "sde/serial/ostream.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename OArchiveT> class associative_oarchive;

template <typename OArchiveT> struct oarchive_traits<associative_oarchive<OArchiveT>>
{
  using stream_type = typename oarchive_traits<OArchiveT>::stream_type;
};

template <typename OArchiveT> class associative_oarchive : public oarchive<associative_oarchive<OArchiveT>>
{
  using ostream_type = typename oarchive_traits<OArchiveT>::stream_type;
  static_assert(!std::is_void_v<ostream_type>);

  using ostream_pos_type = typename ostream_traits<ostream_type>::pos_type;
  using oarchive_base = oarchive<associative_oarchive<OArchiveT>>;
  friend oarchive_base;

public:
  using oarchive_base::operator<<;
  using oarchive_base::operator&;

  ~associative_oarchive()
  {
    if (oar_ == nullptr)
    {
      return;
    }

    ostream_pos_type offset_lookup_start;
    if (!this->stream()->get_position(offset_lookup_start))
    {
      std::abort();
    }

    (*this) << offset_table_.size();
    for (const auto& [key, offset] : offset_table_)
    {
      (*this) << make_packet(std::addressof(key));
      (*this) << make_packet(std::addressof(offset));
    }

    if (!this->stream()->set_position(offset_start_))
    {
      std::abort();
    }
    else if (auto ok_or_error = (*oar_) << make_packet(std::addressof(offset_lookup_start)); !ok_or_error)
    {
      std::abort();
    }
  }

  static expected<associative_oarchive, oarchive_error> create(OArchiveT& original_oar)
  {
    associative_oarchive oar{original_oar};
    if (!oar.stream()->get_position(oar.offset_start_))
    {
      return make_unexpected(oarchive_error::kStreamError);
    }
    else if (auto ok_or_error = (oar << make_packet(std::addressof(oar.offset_start_))); !ok_or_error)
    {
      return make_unexpected(ok_or_error.error());
    }
    return {std::move(oar)};
  }

  associative_oarchive(associative_oarchive&& other) { this->swap(other); }

  associative_oarchive& operator=(associative_oarchive&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(associative_oarchive& other)
  {
    std::swap(this->parent_hash_, other.parent_hash_);
    std::swap(this->offset_start_, other.offset_start_);
    std::swap(this->offset_table_, other.offset_table_);
    std::swap(this->oar_, other.oar_);
  }

  std::size_t key_count() const { return offset_table_.size(); }

private:
  explicit associative_oarchive(OArchiveT& oar) : oar_{std::addressof(oar)} {}

  associative_oarchive(const associative_oarchive& other) = delete;
  associative_oarchive& operator=(const associative_oarchive& other) = delete;

  ostream_type* stream_impl() { return oar_->stream(); }
  const ostream_type* stream_impl() const { return oar_->stream(); }

  template <typename ValueT> constexpr expected<void, oarchive_error> write_impl(const named<ValueT>& named_value)
  {
    const auto name_hash = ComputeHash(std::string_view{named_value.name});
    const auto type_hash = ComputeHash(sizeof(ValueT));

    if (ostream_pos_type offset = {}; !this->stream()->get_position(offset))
    {
      return make_unexpected(oarchive_error::kStreamError);
    }
    else if (const auto [_, added] = offset_table_.emplace(
               std::piecewise_construct,
               std::forward_as_tuple(parent_hash_, name_hash, type_hash),
               std::forward_as_tuple(offset));
             !added)
    {
      (void)_;
      return make_unexpected(oarchive_error::kKeyRepeated);
    }

    const auto previous_parent_hash = parent_hash_;
    parent_hash_ += name_hash;
    auto ok_or_error = ((*this) << named_value.value);
    parent_hash_ = previous_parent_hash;
    return ok_or_error;
  }

  template <typename IteratorT> constexpr expected<void, oarchive_error> write_impl(const sequence<IteratorT>& sequence)
  {
    std::size_t index = 0;
    for (const auto& element : sequence)
    {
      if (auto ok_or_error = (*this) << named{format("element[%lu]", index++), element}; !ok_or_error)
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <typename PointerT> constexpr expected<void, oarchive_error> write_impl(const basic_packet<PointerT>& packet)
  {
    return (*oar_) << packet;
  }

  template <typename PointerT, std::size_t Len>
  constexpr expected<void, oarchive_error> write_impl(const basic_packet_fixed_size<PointerT, Len>& packet)
  {
    return (*oar_) << packet;
  }

  using tiered_hash = std::tuple<Hash, Hash, Hash>;
  struct tiered_hash_value
  {
    std::size_t operator()(const tiered_hash& dh) const
    {
      return (std::get<0>(dh) + std::get<1>(dh), std::get<2>(dh)).value;
    }
  };

  Hash parent_hash_ = {};
  ostream_pos_type offset_start_ = {};
  std::unordered_map<tiered_hash, ostream_pos_type, tiered_hash_value> offset_table_;

  OArchiveT* oar_ = nullptr;
};

template <typename OArchiveT> associative_oarchive(OArchiveT& oar) -> associative_oarchive<OArchiveT>;

struct save_trivial_associative_oarchive
{
  template <typename OStreamT, typename ValueT> void operator()(associative_oarchive<OStreamT>& ar, const ValueT& value)
  {
    ar << make_packet(std::addressof(value));
  }
};

template <typename OArchiveT, typename ValueT>
struct save_impl<associative_oarchive<OArchiveT>, ValueT>
    : std::conditional_t<
        (is_trivially_serializable_v<associative_oarchive<OArchiveT>, ValueT> and
         !save_is_implemented_v<associative_oarchive<OArchiveT>, ValueT>),
        save_trivial_associative_oarchive,
        save<associative_oarchive<OArchiveT>, ValueT>>
{};

template <typename OArchiveT>
expected<associative_oarchive<OArchiveT>, oarchive_error> make_associative(oarchive<OArchiveT>& oar)
{
  return associative_oarchive<OArchiveT>::create(static_cast<OArchiveT&>(oar));
}

}  // namespace sde::serial
