/**
 * @copyright 2022-present Brian Cairl
 *
 * @file mem_ostream.cpp
 */

// C++ Standard Library
#include <utility>

// SDE
#include "sde/serial/mem_ostream.hpp"

namespace sde::serial
{

mem_ostream::mem_ostream(const std::size_t initial_capacity) { buffer_.reserve(initial_capacity); }

mem_ostream::mem_ostream(mem_ostream&& other) : buffer_{std::move(other.buffer_)} {}

mem_ostream::~mem_ostream() = default;

}  // namespace sde::serial