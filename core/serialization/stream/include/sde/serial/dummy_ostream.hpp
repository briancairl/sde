/**
 * @copyright 2022-present Brian Cairl
 *
 * @file dummy_ostream.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>

// SDE
#include "sde/serial/ostream.hpp"

namespace sde::serial
{

class dummy_ostream final : public ostream<dummy_ostream>
{
  friend class ostream<dummy_ostream>;

public:
  dummy_ostream() = default;
  dummy_ostream(dummy_ostream&& other) = default;
  dummy_ostream& operator=(dummy_ostream&& other) = default;

  ~dummy_ostream();

  constexpr std::size_t offset() const { return offset_; }

  void swap(dummy_ostream& other) { std::swap(this->offset_, other.offset_); }

private:
  /**
   * @copydoc ostream<dummy_ostream>::write
   */
  std::size_t write_impl([[maybe_unused]] const void* ptr, std::size_t len)
  {
    offset_ += len;
    return len;
  }

  /// Byte stream buffer
  std::size_t offset_ = 0UL;
};

}  // namespace sde::serial
