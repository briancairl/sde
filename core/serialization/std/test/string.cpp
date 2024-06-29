/**
 * @copyright 2023-present Brian Cairl
 *
 * @file string.cpp
 */

// C++ Standard Library
#include <string>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/binary_archive.hpp"
#include "sde/serial/mem_stream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/std/string.hpp"

using namespace sde::serial;


TEST(StdString, EmptyString)
{
  const std::string kExpected = {};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::string read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}

TEST(StdString, NonEmptyString)
{
  const std::string kExpected = "expected";

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::string read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}
