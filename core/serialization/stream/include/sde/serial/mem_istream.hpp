/**
 * @copyright 2022-present Brian Cairl
 *
 * @file mem_istream.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <cstring>
#include <vector>

// SDE
#include "sde/memory.hpp"
#include "sde/serial/istream.hpp"

namespace sde::serial
{

template <typename ChunkT, typename AllocatorT> class mem_ostream;

template <typename ChunkT = std::uint8_t, typename AllocatorT = allocator<ChunkT>>
class mem_istream final : public istream<mem_istream<ChunkT, AllocatorT>>
{
  friend class istream<mem_istream<ChunkT, AllocatorT>>;

public:
  ~mem_istream() = default;

  mem_istream(mem_ostream<ChunkT, AllocatorT>&& other) : buffer_{std::move(other.buffer_)}, pos_{0} {}

  mem_istream(std::vector<ChunkT, AllocatorT>&& buffer) : buffer_{std::move(buffer)}, pos_{0} {}

  mem_istream(mem_istream&& other) { this->swap(other); }

  mem_istream& operator=(mem_istream&& other)
  {
    this->swap(other);
    return *this;
  }

  void swap(mem_istream& other)
  {
    std::swap(this->buffer_, other.buffer_);
    std::swap(this->pos_, other.pos_);
  }

private:
  mem_istream(const mem_istream& other) = delete;
  mem_istream& operator=(const mem_istream& other) = delete;

  /**
   * @copydoc istream<mem_istream>::read
   */
  std::size_t read_impl(void* ptr, std::size_t len)
  {
    const std::size_t bump_len = (len / sizeof(ChunkT)) + (((len % sizeof(ChunkT)) == 0) ? 0 : 1);
    std::memcpy(ptr, reinterpret_cast<void*>(buffer_.data() + pos_), len);
    pos_ += bump_len;
    return len;
  }

  /**
   * @copydoc istream<mem_istream>::peek
   */
  ChunkT peek_impl() { return buffer_[pos_]; }

  /**
   * @copydoc istream<mem_istream>::available
   */
  std::size_t available_impl() const { return buffer_.size() - pos_; }

  /// Byte stream buffer
  std::vector<ChunkT, AllocatorT> buffer_ = {};

  /// Current read-byte position
  std::size_t pos_ = 0;
};

}  // namespace sde::serial
