/**
 * @copyright 2023-present Brian Cairl
 *
 * @file chrono.cpp
 */

// C++ Standard Library
#include <chrono>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/binary_iarchive.hpp"
#include "sde/serial/binary_oarchive.hpp"
#include "sde/serial/mem_istream.hpp"
#include "sde/serial/mem_ostream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/std/chrono.hpp"

using namespace sde::serial;


TEST(StdChrono, Duration)
{
  const auto kExpected = std::chrono::milliseconds{123};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::chrono::milliseconds read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}

TEST(StdChrono, TimePoint)
{
  const std::chrono::steady_clock::time_point kExpected{std::chrono::milliseconds{123}};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::chrono::steady_clock::time_point read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}
