/**
 * @copyright 2022-present Brian Cairl
 *
 * @file ostream.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>

// SDE
#include "sde/crtp.hpp"

namespace sde::serial
{

template <typename OStreamT> class ostream : public crtp_base<ostream<OStreamT>>
{
public:
  /**
   * @brief Writes bytes to the stream
   */
  constexpr std::size_t write(const void* ptr, std::size_t len) { return this->derived().write_impl(ptr, len); }

  /**
   * @brief Writes C-style array to the stream
   */
  template <typename ElementT, std::size_t ElementCount>
  constexpr std::size_t write(const ElementT (&array)[ElementCount])
  {
    return ostream::write(reinterpret_cast<const void*>(array), sizeof(ElementT) * ElementCount);
  }

  /**
   * @brief Flushes buffered bytes to target
   */
  constexpr void flush() { return this->derived().flush_impl(); }

  ostream() = default;

private:
  ostream(const ostream&) = default;

  static constexpr void flush_impl()
  { /*default*/
  }
};

}  // namespace sde::serial
