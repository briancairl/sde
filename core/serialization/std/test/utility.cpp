/**
 * @copyright 2023-present Brian Cairl
 *
 * @file utility.cpp
 */

// C++ Standard Library
#include <string>
#include <utility>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/binary_archive.hpp"
#include "sde/serial/mem_stream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/serial/std/utility.hpp"

using namespace sde::serial;


TEST(StdUtility, TrivialPair)
{
  const std::pair<int, double> kExpected{1, 1.3};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::pair<int, double> read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}

TEST(StdUtility, NonTrivialPair)
{
  const std::pair<std::string, double> kExpected{std::to_string(1), 1.3};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::pair<std::string, double> read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}
