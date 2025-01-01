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

template <typename OStreamT> struct ostream_traits;

template <typename OStreamT> class ostream : public crtp_base<ostream<OStreamT>>
{
public:
  using pos_type = typename ostream_traits<OStreamT>::pos_type;

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

  /**
   * @brief Gets current stream position
   */
  constexpr bool get_position(pos_type& pos) const { return this->derived().get_position_impl(pos); }

  /**
   * @brief Sets current stream position
   */
  constexpr bool set_position(const pos_type& pos) { return this->derived().set_position_impl(pos); }

  ostream() = default;

private:
  ostream(const ostream&) = delete;
  ostream& operator=(const ostream&) = delete;

  static constexpr void flush_impl()
  { /*default*/
  }
};

}  // namespace sde::serial
