/**
 * @copyright 2022-present Brian Cairl
 *
 * @file istream.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>

// SDE
#include "sde/crtp.hpp"

namespace sde::serial
{

template <typename IStreamT> struct istream_traits;

template <typename IStreamT> class istream : public crtp_base<istream<IStreamT>>
{
public:
  using pos_type = typename istream_traits<IStreamT>::pos_type;

  /**
   * @brief Reads bytes from the stream
   */
  constexpr std::size_t read(void* ptr, std::size_t len) { return this->derived().read_impl(ptr, len); }

  /**
   * @brief Reads stream to C-style array
   */
  template <typename ElementT, std::size_t ElementCount> constexpr std::size_t read(ElementT (&array)[ElementCount])
  {
    return istream::read(reinterpret_cast<void*>(array), sizeof(ElementT) * ElementCount);
  }

  /**
   * @brief Returns number of available bytes left in the stream
   */
  constexpr std::size_t available() const { return this->derived().available_impl(); }

  /**
   * @brief Gets current stream position
   */
  constexpr bool get_position(pos_type& pos) const { return this->derived().get_position_impl(pos); }

  /**
   * @brief Sets current stream position
   */
  constexpr bool set_position(const pos_type& pos) { return this->derived().set_position_impl(pos); }

  istream() = default;

private:
  istream(const istream&) = delete;
  istream& operator=(const istream&) = delete;

  static constexpr void flush_impl()
  { /*default*/
  }
};

}  // namespace sde::serial
