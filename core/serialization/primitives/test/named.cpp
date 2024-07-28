/**
 * @copyright 2022-present Brian Cairl
 *
 * @file named.cpp
 */

// C++ Standard Library
#include <cstring>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/binary_iarchive.hpp"
#include "sde/serial/binary_oarchive.hpp"
#include "sde/serial/file_istream.hpp"
#include "sde/serial/file_ostream.hpp"
#include "sde/serial/named.hpp"
#include "sde/serial/packet.hpp"

using namespace sde::serial;


TEST(Named, PrimitiveElementValue)
{
  static const float TARGET_VALUE = 123.f;

  {
    auto ofs = file_ostream::create("Named.PrimitiveElementValue.bin").value();
    binary_oarchive oar{ofs};
    const float v = TARGET_VALUE;
    ASSERT_NO_THROW((oar << named{"value", v}));
  }

  {
    auto ifs = file_istream::create("Named.PrimitiveElementValue.bin").value();
    binary_iarchive iar{ifs};
    float v;
    ASSERT_NO_THROW((iar >> named{"value", v}));
    ASSERT_EQ(v, TARGET_VALUE);
  }
}

struct Trivial
{
  int x;
  float y, z;
};

template <typename Archive> struct save<Archive, ::Trivial>
{
  void operator()(Archive& ar, const ::Trivial& target) { ar << make_packet(&target); }
};

template <typename Archive> struct load<Archive, ::Trivial>
{
  void operator()(Archive& ar, ::Trivial& target) { ar >> make_packet(&target); }
};

TEST(Named, TrivialValue)
{
  static const Trivial TARGET_VALUE = {1, 123.f, 321.f};

  {
    auto ofs = file_ostream::create("Named.TrivialValue.bin").value();
    binary_oarchive oar{ofs};
    const Trivial v = TARGET_VALUE;
    ASSERT_NO_THROW((oar << named{"value", v}));
  }

  {
    auto ifs = file_istream::create("Named.TrivialValue.bin").value();
    binary_iarchive iar{ifs};
    Trivial v;
    ASSERT_NO_THROW((iar >> named{"value", v}));
    ASSERT_EQ(std::memcmp(&v, &TARGET_VALUE, sizeof(TARGET_VALUE)), 0);
  }
}
