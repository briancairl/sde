/**
 * @copyright 2024-present Brian Cairl
 *
 * @file hash_archive.hpp
 */

// C++ Standard Library
#include <vector>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/hash_oarchive.hpp"

using namespace sde::serial;

struct TrivialStruct
{
  int x;
  float y;
  double z;
};

struct NonTrivialStruct
{
  std::vector<int> values;
};

namespace sde::serial
{

template <typename Archive> struct load<Archive, ::TrivialStruct>
{
  void operator()(Archive& ar, ::TrivialStruct& target) { ar >> make_packet(&target); }
};

template <typename Archive> struct save<Archive, ::TrivialStruct>
{
  void operator()(Archive& ar, const ::TrivialStruct& target) { ar << make_packet(&target); }
};

template <typename OArchive> struct save<OArchive, ::NonTrivialStruct>
{
  void operator()(OArchive& ar, const ::NonTrivialStruct& target)
  {
    ar << named{"len", target.values.size()};
    for (auto& v : target.values)
    {
      ar << v;
    }
  }
};

template <typename IArchive> struct load<IArchive, ::NonTrivialStruct>
{
  void operator()(IArchive& ar, ::NonTrivialStruct& target)
  {
    std::size_t len;
    ar >> named{"len", len};

    target.values.resize(len);
    for (auto& v : target.values)
    {
      ar >> v;
    }
  }
};

}  // namespace sde::serial

TEST(HashOArchive, PrimitiveValue)
{
  hash_oarchive oar;

  float primitive = 123.f;
  ASSERT_NO_THROW(oar << primitive);
  ASSERT_NE(oar.digest().value, 0UL) << oar.digest();
}

TEST(HashOArchive, TrivialValue)
{
  hash_oarchive oar;

  TrivialStruct trivial_value;
  ASSERT_NO_THROW(oar << trivial_value);
  ASSERT_NE(oar.digest().value, 0UL) << oar.digest();
}

TEST(HashOArchive, NonTrivialStruct)
{
  hash_oarchive oar;

  NonTrivialStruct non_trivial_value;
  ASSERT_NO_THROW(oar << non_trivial_value);
  ASSERT_NE(oar.digest().value, 0UL) << oar.digest();
}
