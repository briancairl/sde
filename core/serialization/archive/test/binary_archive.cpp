/**
 * @copyright 2022-present Brian Cairl
 *
 * @file binary_archive.hpp
 */

// C++ Standard Library
#include <vector>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/binary_iarchive.hpp"
#include "sde/serial/binary_oarchive.hpp"
#include "sde/serial/file_istream.hpp"
#include "sde/serial/file_ostream.hpp"

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
    ar << target.values.size();
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
    ar >> len;

    target.values.resize(len);
    for (auto& v : target.values)
    {
      ar >> v;
    }
  }
};

}  // namespace sde::serial

TEST(BinaryOArchive, PrimitiveValue)
{
  auto ofs = file_ostream::create("BinaryOArchive.PrimitiveValue.bin").value();
  binary_oarchive oar{ofs};

  float primitive = 123.f;
  ASSERT_NO_THROW(oar << primitive);
}

TEST(BinaryOArchive, TrivialValue)
{
  auto ofs = file_ostream::create("BinaryOArchive.TrivialValue.bin").value();
  binary_oarchive oar{ofs};

  TrivialStruct trivial_value;
  ASSERT_NO_THROW(oar << trivial_value);
}

TEST(BinaryOArchive, NonTrivialStruct)
{
  auto ofs = file_ostream::create("BinaryOArchive.NonTrivialStruct.bin").value();
  binary_oarchive oar{ofs};

  NonTrivialStruct non_trivial_value;
  ASSERT_NO_THROW(oar << non_trivial_value);
}


TEST(BinaryIArchive, ReadbackTrivialStruct)
{
  const TrivialStruct target_trivial_value{1, 2, 3};

  {
    auto ofs = file_ostream::create("BinaryOArchive.ReadbackTrivialStruct.bin").value();
    binary_oarchive oar{ofs};
    ASSERT_NO_THROW(oar << target_trivial_value);
  }

  {
    auto ifs = file_istream::create("BinaryOArchive.ReadbackTrivialStruct.bin").value();
    binary_iarchive iar{ifs};

    TrivialStruct read_trivial_value;
    ASSERT_NO_THROW(iar >> read_trivial_value);

    ASSERT_EQ(read_trivial_value.x, target_trivial_value.x);
    ASSERT_EQ(read_trivial_value.y, target_trivial_value.y);
    ASSERT_EQ(read_trivial_value.z, target_trivial_value.z);
  }
}

TEST(BinaryIArchive, ReadbackNonTrivialStruct)
{
  const NonTrivialStruct target_non_trivial_value{{1, 2, 3}};

  ASSERT_GT(target_non_trivial_value.values.size(), 0UL);

  {
    auto ofs = file_ostream::create("BinaryOArchive.ReadbackNonTrivialStruct.bin").value();
    binary_oarchive oar{ofs};
    ASSERT_NO_THROW(oar << target_non_trivial_value);
  }

  {
    auto ifs = file_istream::create("BinaryOArchive.ReadbackNonTrivialStruct.bin").value();
    binary_iarchive iar{ifs};

    NonTrivialStruct read_non_trivial_value;
    ASSERT_NO_THROW(iar >> read_non_trivial_value);

    ASSERT_EQ(read_non_trivial_value.values, target_non_trivial_value.values);
  }
}
