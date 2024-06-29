/**
 * @copyright 2023-present Brian Cairl
 *
 * @file vector.cpp
 */

// C++ Standard Library
#include <string>
#include <vector>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/binary_archive.hpp"
#include "sde/serial/mem_stream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/serial/std/vector.hpp"

using namespace sde::serial;


TEST(StdVector, TriviallySerializableElement)
{
  const std::vector<float> kExpected = {1, 2, 3, 4, 5};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::vector<float> read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}

TEST(StdVector, NonTriviallySerializableElement)
{
  const std::vector<std::string> kExpected = {"1", "2", "3", "4", "5"};

  mem_ostream oms{};
  {
    binary_oarchive oar{oms};
    ASSERT_NO_THROW((oar << named{"value", kExpected}));
  }

  mem_istream ims{std::move(oms)};
  {
    binary_iarchive iar{ims};
    std::vector<std::string> read;
    ASSERT_NO_THROW((iar >> named{"value", read}));
    ASSERT_EQ(read, kExpected);
  }
}
