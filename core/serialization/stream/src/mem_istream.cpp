/**
 * @copyright 2022-present Brian Cairl
 *
 * @file mem_istream.cpp
 */

// C++ Standard Library
#include <utility>

// SDE
#include "sde/serial/mem_istream.hpp"
#include "sde/serial/mem_ostream.hpp"

namespace sde::serial
{

mem_istream::mem_istream(std::vector<std::uint8_t>&& buffer) : buffer_{std::move(buffer)}, pos_{0} {}

mem_istream::mem_istream(mem_istream&& other) : buffer_{std::move(other.buffer_)}, pos_{0} {}

mem_istream::mem_istream(mem_ostream&& other) : buffer_{std::move(other.buffer_)}, pos_{0} {}

mem_istream::~mem_istream() = default;

}  // namespace sde::serial