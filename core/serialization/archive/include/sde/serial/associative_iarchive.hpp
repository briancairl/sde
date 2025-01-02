/**
 * @copyright 2025-present Brian Cairl
 *
 * @file associative_iarchive.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>
#include <unordered_map>
#include <utility>

// SDE
#include "sde/format.hpp"
#include "sde/hash.hpp"
#include "sde/serial/iarchive.hpp"
#include "sde/serial/istream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/sequence.hpp"

namespace sde::serial
{

template <typename IArchiveT> class associative_iarchive;

template <typename IArchiveT> struct iarchive_traits<associative_iarchive<IArchiveT>>
{
  using stream_type = typename iarchive_traits<IArchiveT>::stream_type;
};

template <typename IArchiveT> class associative_iarchive : public iarchive<associative_iarchive<IArchiveT>>
{
  using istream_type = typename iarchive_traits<IArchiveT>::stream_type;
  static_assert(!std::is_void_v<istream_type>);

  using istream_pos_type = typename istream_traits<istream_type>::pos_type;
  using iarchive_base = iarchive<associative_iarchive<IArchiveT>>;
  friend iarchive_base;

public:
  using iarchive_base::operator>>;
  using iarchive_base::operator&;

  ~associative_iarchive() = default;

  static expected<associative_iarchive, iarchive_error> create(IArchiveT& original_iar)
  {
    associative_iarchive iar{original_iar};

    istream_pos_type offset_data_start = {};

    {
      // Read position where offset lookup table starts
      istream_pos_type offset_lookup_start = {};
      if (auto ok_or_error = (iar >> make_packet(std::addressof(offset_lookup_start))); !ok_or_error)
      {
        return make_unexpected(ok_or_error.error());
      }

      // Get position in stream where value data starts
      if (!iar.stream()->get_position(offset_data_start))
      {
        return make_unexpected(iarchive_error::kStreamError);
      }

      // Jump to offset lookup table
      if (!iar.stream()->set_position(offset_lookup_start))
      {
        return make_unexpected(iarchive_error::kStreamError);
      }
    }

    {
      // Read offset lookup table
      std::size_t kv_count = 0;
      if (auto ok_or_error = (iar >> kv_count); !ok_or_error)
      {
        return make_unexpected(ok_or_error.error());
      }
      iar.offset_table_.reserve(kv_count);
      for (std::size_t i = 0; i < kv_count; ++i)
      {
        tiered_hash key = {};
        iar >> make_packet(std::addressof(key));
        istream_pos_type offset = {};
        iar >> make_packet(std::addressof(offset));
        iar.offset_table_.emplace(key, offset);
      }
    }

    // Jump back to where data starts
    if (!iar.stream()->set_position(offset_data_start))
    {
      return make_unexpected(iarchive_error::kStreamError);
    }
    return {std::move(iar)};
  }

  associative_iarchive(associative_iarchive&& other) { this->swap(other); }

  associative_iarchive& operator=(associative_iarchive&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(associative_iarchive& other)
  {
    std::swap(this->parent_hash_, other.parent_hash_);
    std::swap(this->offset_table_, other.offset_table_);
    std::swap(this->iar_, other.iar_);
  }

  std::size_t key_count() const { return offset_table_.size(); }

private:
  explicit associative_iarchive(IArchiveT& oar) : iar_{std::addressof(oar)} {};

  associative_iarchive(const associative_iarchive& other) = delete;
  associative_iarchive& operator=(const associative_iarchive& other) = delete;

  istream_type* stream_impl() { return iar_->stream(); }
  const istream_type* stream_impl() const { return iar_->stream(); }

  template <typename ValueT> constexpr expected<void, iarchive_error> read_impl(named<ValueT> named_value)
  {
    const auto name_hash = ComputeHash(std::string_view{named_value.name});
    const auto type_hash = ComputeHash(sizeof(ValueT));

    const auto key = tiered_hash{parent_hash_, name_hash, type_hash};
    const auto offset_itr = offset_table_.find(key);
    if (offset_itr == std::end(offset_table_))
    {
      return make_unexpected(iarchive_error::kKeyMissing);
    }
    else if (!this->stream()->set_position(offset_itr->second))
    {
      return make_unexpected(iarchive_error::kStreamError);
    }

    const auto previous_parent_hash = parent_hash_;
    parent_hash_ += name_hash;
    auto ok_or_error = ((*this) >> named_value.value);
    parent_hash_ = previous_parent_hash;
    return ok_or_error;
  }

  template <typename IteratorT> constexpr expected<void, iarchive_error> read_impl(sequence<IteratorT> sequence)
  {
    std::size_t index = 0;
    for (auto& element : sequence)
    {
      if (auto ok_or_error = (*this) >> named{format("element[%lu]", index++), element}; !ok_or_error)
      {
        return ok_or_error;
      }
    }
    return {};
  }

  template <typename PointerT> constexpr expected<void, iarchive_error> read_impl(basic_packet<PointerT> packet)
  {
    return (*iar_) >> packet;
  }

  template <typename PointerT, std::size_t Len>
  constexpr expected<void, iarchive_error> read_impl(basic_packet_fixed_size<PointerT, Len> packet)
  {
    return (*iar_) >> packet;
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
  std::unordered_map<tiered_hash, istream_pos_type, tiered_hash_value> offset_table_;
  IArchiveT* iar_ = nullptr;
};

template <typename IArchiveT> associative_iarchive(IArchiveT& oar) -> associative_iarchive<IArchiveT>;

struct load_trivial_associative_iarchive
{
  template <typename OStreamT, typename ValueT> void operator()(associative_iarchive<OStreamT>& ar, ValueT& value)
  {
    ar >> make_packet(std::addressof(value));
  }
};

template <typename IArchiveT, typename ValueT>
struct load_impl<associative_iarchive<IArchiveT>, ValueT>
    : std::conditional_t<
        (is_trivially_serializable_v<associative_iarchive<IArchiveT>, ValueT> and
         !load_is_implemented_v<associative_iarchive<IArchiveT>, ValueT>),
        load_trivial_associative_iarchive,
        load<associative_iarchive<IArchiveT>, ValueT>>
{};

template <typename IArchiveT>
expected<associative_iarchive<IArchiveT>, iarchive_error> make_associative(iarchive<IArchiveT>& iar)
{
  return associative_iarchive<IArchiveT>::create(static_cast<IArchiveT&>(iar));
}

}  // namespace sde::serial
