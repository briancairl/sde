/**
 * @copyright 2022-present Brian Cairl
 *
 * @file mem_ostream.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <cstring>
#include <vector>

// SDE
#include "sde/memory.hpp"
#include "sde/serial/ostream.hpp"

namespace sde::serial
{

template <typename ChunkT, typename AllocatorT> class mem_istream;

template <typename ChunkT, typename AllocatorT> class mem_ostream;

template <typename ChunkT, typename AllocatorT> struct ostream_traits<mem_ostream<ChunkT, AllocatorT>>
{
  using pos_type = std::size_t;
};

template <typename ChunkT = std::uint8_t, typename AllocatorT = allocator<ChunkT>>
class mem_ostream final : public ostream<mem_ostream<ChunkT, AllocatorT>>
{
  friend class ostream<mem_ostream<ChunkT, AllocatorT>>;
  friend class mem_istream<ChunkT, AllocatorT>;

public:
  ~mem_ostream() = default;

  mem_ostream(const std::size_t initial_capacity = 64UL) : buffer_{}, pos_{0} { buffer_.reserve(initial_capacity); }

  mem_ostream(mem_ostream&& other) { this->swap(other); }

  mem_ostream& operator=(mem_ostream&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(mem_ostream& other)
  {
    std::swap(this->buffer_, other.buffer_);
    std::swap(this->pos_, other.pos_);
  }

private:
  mem_ostream(const mem_ostream& other) = delete;
  mem_ostream& operator=(const mem_ostream& other) = delete;

  /**
   * @copydoc ostream<mem_ostream>::write
   */
  std::size_t write_impl(const void* ptr, std::size_t len)
  {
    const std::size_t bump_len = (len / sizeof(ChunkT)) + (((len % sizeof(ChunkT)) == 0) ? 0 : 1);
    const std::size_t next_pos = pos_ + bump_len;
    if (next_pos > buffer_.size())
    {
      buffer_.resize(next_pos);
    }
    std::memcpy(reinterpret_cast<void*>(buffer_.data() + pos_), ptr, len);
    pos_ = next_pos;
    return len;
  }

  /**
   * @copydoc ostream<mem_ostream>::get_position
   */
  bool get_position_impl(std::size_t& pos) const
  {
    pos = pos_;
    return !buffer_.empty();
  }

  /**
   * @copydoc ostream<mem_ostream>::set_position
   */
  bool set_position_impl(const std::size_t& pos)
  {
    if (pos < buffer_.size())
    {
      pos_ = pos;
      return true;
    }
    return false;
  }

  /// Byte stream buffer
  std::vector<ChunkT, AllocatorT> buffer_ = {};

  /// Current write position
  std::size_t pos_ = 0;
};

}  // namespace sde::serial
