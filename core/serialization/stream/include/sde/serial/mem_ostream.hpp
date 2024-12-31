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

template <typename ChunkT = std::uint8_t, typename AllocatorT = allocator<ChunkT>>
class mem_ostream final : public ostream<mem_ostream<ChunkT, AllocatorT>>
{
  friend class ostream<mem_ostream<ChunkT, AllocatorT>>;
  friend class mem_istream<ChunkT, AllocatorT>;

public:
  ~mem_ostream() = default;

  mem_ostream(const std::size_t initial_capacity = 64UL) { buffer_.reserve(initial_capacity); }

  mem_ostream(mem_ostream&& other) { this->swap(other); }

  mem_ostream& operator=(mem_ostream&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(mem_ostream& other) { std::swap(this->buffer_, other.buffer_); }

private:
  mem_ostream(const mem_ostream& other) = delete;
  mem_ostream& operator=(const mem_ostream& other) = delete;

  /**
   * @copydoc ostream<mem_ostream>::write
   */
  std::size_t write_impl(const void* ptr, std::size_t len)
  {
    const std::size_t bump_len = (len / sizeof(ChunkT)) + (((len % sizeof(ChunkT)) == 0) ? 0 : 1);
    const std::size_t pos = buffer_.size();
    buffer_.resize(buffer_.size() + bump_len);
    std::memcpy(reinterpret_cast<void*>(buffer_.data() + pos), ptr, len);
    return len;
  }

  /// Byte stream buffer
  std::vector<ChunkT, AllocatorT> buffer_ = {};
};

}  // namespace sde::serial
