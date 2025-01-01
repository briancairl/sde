/**
 * @copyright 2025-present Brian Cairl
 *
 * @file associative_archive.cpp
 */

// C++ Standard Library
#include <vector>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/associative_archive.hpp"
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
  TrivialStruct a;
  TrivialStruct b;
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
    ar << named{"a", target.a};
    ar << named{"b", target.b};
  }
};

template <typename IArchive> struct load<IArchive, ::NonTrivialStruct>
{
  void operator()(IArchive& ar, ::NonTrivialStruct& target)
  {
    ar >> named{"a", target.a};
    ar >> named{"b", target.b};
  }
};

}  // namespace sde::serial

TEST(AssociativeOArchive, PrimitiveValue)
{
  auto ofs = file_ostream::create("AssociativeOArchive.PrimitiveValue.bin").value();
  binary_oarchive oar{ofs};
  auto assoc_oar_or_error = make_associative(oar);
  auto& assoc_oar = assoc_oar_or_error.value();

  float primitive = 123.f;
  ASSERT_TRUE((assoc_oar << named{"primitive", primitive}));
  ASSERT_FALSE((assoc_oar << named{"primitive", primitive}));
}

TEST(AssociativeOArchive, TrivialValue)
{
  auto ofs = file_ostream::create("AssociativeOArchive.TrivialValue.bin").value();
  binary_oarchive oar{ofs};
  auto assoc_oar_or_error = make_associative(oar);
  auto& assoc_oar = assoc_oar_or_error.value();

  TrivialStruct trivial_value;
  ASSERT_TRUE((assoc_oar << named{"trivial", trivial_value}));
}

TEST(AssociativeOArchive, NonTrivialStruct)
{
  auto ofs = file_ostream::create("AssociativeOArchive.NonTrivialStruct.bin").value();
  binary_oarchive oar{ofs};
  auto assoc_oar_or_error = make_associative(oar);
  auto& assoc_oar = assoc_oar_or_error.value();

  NonTrivialStruct non_trivial_value;
  ASSERT_TRUE((assoc_oar << named{"non_trivial_value", non_trivial_value}));
}


TEST(AssociativeIArchive, ReadbackTrivialStruct)
{
  const TrivialStruct target_trivial_value{1, 2, 3};

  {
    auto ofs = file_ostream::create("AssociativeOArchive.ReadbackTrivialStruct.bin").value();
    binary_oarchive oar{ofs};
    auto assoc_oar_or_error = make_associative(oar);
    auto& assoc_oar = assoc_oar_or_error.value();
    ASSERT_TRUE((assoc_oar << named{"target_trivial_value", target_trivial_value}));
  }

  {
    auto ifs = file_istream::create("AssociativeOArchive.ReadbackTrivialStruct.bin").value();
    binary_iarchive iar{ifs};

    auto assoc_iar_or_error = make_associative(iar);
    auto& assoc_iar = assoc_iar_or_error.value();

    TrivialStruct read_trivial_value;
    ASSERT_TRUE((assoc_iar >> named{"target_trivial_value", read_trivial_value}));

    ASSERT_EQ(read_trivial_value.x, target_trivial_value.x);
    ASSERT_EQ(read_trivial_value.y, target_trivial_value.y);
    ASSERT_EQ(read_trivial_value.z, target_trivial_value.z);
  }
}

TEST(AssociativeIArchive, ReadbackNonTrivialStruct)
{
  const NonTrivialStruct target_non_trivial_value{TrivialStruct{1, 2, 3}, TrivialStruct{3, 4, 5}};

  std::size_t expected_key_count = 0;
  {
    auto ofs = file_ostream::create("AssociativeOArchive.ReadbackNonTrivialStruct.bin").value();
    binary_oarchive oar{ofs};
    auto assoc_oar_or_error = make_associative(oar);
    auto& assoc_oar = assoc_oar_or_error.value();
    ASSERT_TRUE((assoc_oar << named{"target_non_trivial_value_a", target_non_trivial_value}));
    ASSERT_TRUE((assoc_oar << named{"target_non_trivial_value_b", target_non_trivial_value}));
    expected_key_count = assoc_oar.key_count();
  }

  {
    auto ifs = file_istream::create("AssociativeOArchive.ReadbackNonTrivialStruct.bin").value();
    binary_iarchive iar{ifs};

    auto assoc_iar_or_error = make_associative(iar);
    auto& assoc_iar = assoc_iar_or_error.value();

    ASSERT_EQ(expected_key_count, assoc_iar.key_count());

    NonTrivialStruct read_non_trivial_value;
    ASSERT_TRUE((assoc_iar >> named{"target_non_trivial_value_b", read_non_trivial_value}));
    ASSERT_TRUE((assoc_iar >> named{"target_non_trivial_value_a", read_non_trivial_value}));

    ASSERT_EQ(read_non_trivial_value.a.x, target_non_trivial_value.a.x);
    ASSERT_EQ(read_non_trivial_value.a.y, target_non_trivial_value.a.y);
    ASSERT_EQ(read_non_trivial_value.a.z, target_non_trivial_value.a.z);
    ASSERT_EQ(read_non_trivial_value.b.x, target_non_trivial_value.b.x);
    ASSERT_EQ(read_non_trivial_value.b.y, target_non_trivial_value.b.y);
    ASSERT_EQ(read_non_trivial_value.b.z, target_non_trivial_value.b.z);
  }
}