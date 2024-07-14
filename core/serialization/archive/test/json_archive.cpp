/**
 * @copyright 2022-present Brian Cairl
 *
 * @file json_archive.hpp
 */

// C++ Standard Library
#include <cstring>
#include <vector>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/file_istream.hpp"
#include "sde/serial/file_ostream.hpp"
#include "sde/serial/json_iarchive.hpp"
#include "sde/serial/json_oarchive.hpp"

using namespace sde::serial;

struct TrivialStruct
{
  int x;
  float y;
  double z;
};

static bool operator==(const TrivialStruct& lhs, const TrivialStruct& rhs)
{
  return std::memcmp(&lhs, &rhs, sizeof(TrivialStruct)) == 0;
}

struct TrivialNestedStruct
{
  std::string label_1;
  std::string label_2;
  TrivialStruct first;
  TrivialStruct second;
};

static bool operator==(const TrivialNestedStruct& lhs, const TrivialNestedStruct& rhs)
{
  return (lhs.label_1 == rhs.label_1) and (lhs.label_2 == rhs.label_2) and (lhs.first == rhs.first) and
    (lhs.second == rhs.second);
}

namespace sde::serial
{

template <typename Archive> struct serialize<Archive, ::TrivialStruct>
{
  void operator()(Archive& ar, ::TrivialStruct& value)
  {
    ar& named{"x", value.x};
    ar& named{"y", value.y};
    ar& named{"z", value.z};
  }
};

template <typename Archive> struct serialize<Archive, ::TrivialNestedStruct>
{
  void operator()(Archive& ar, ::TrivialNestedStruct& value)
  {
    ar& named{"label_1", value.label_1};
    ar& named{"label_2", value.label_2};
    ar& named{"first", value.first};
    ar& named{"second", value.second};
  }
};

/**
 * @brief Archive-generic <code>std::vector<ValueT, AllocT></code> save implementation
 */
template <typename OArchive, typename ValueT, typename AllocT> struct save<OArchive, std::vector<ValueT, AllocT>>
{
  void operator()(OArchive& ar, const std::vector<ValueT, AllocT>& vec)
  {
    ar << named{"size", vec.size()};
    if constexpr (is_trivially_serializable_v<OArchive, ValueT>)
    {
      ar << named{"data", make_packet(vec.data(), vec.size())};
    }
    else
    {
      ar << named{"data", make_sequence(vec.begin(), vec.end())};
    }
  }
};

/**
 * @brief Archive-generic <code>std::vector<ValueT, AllocT></code> load implementation
 */
template <typename IArchive, typename ValueT, typename AllocT> struct load<IArchive, std::vector<ValueT, AllocT>>
{
  void operator()(IArchive& ar, std::vector<ValueT, AllocT>& vec)
  {
    {
      std::size_t size;
      ar >> named{"size", size};
      vec.resize(size);
    }

    if constexpr (is_trivially_serializable_v<IArchive, ValueT>)
    {
      ar >> named{"data", make_packet(vec.data(), vec.size())};
    }
    else
    {
      ar >> named{"data", make_sequence(vec.begin(), vec.end())};
    }
  }
};

}  // namespace sde::serial

TEST(JSONOArchive, Primitive)
{
  file_handle_ostream ofs{stdout};
  json_oarchive oar{ofs};

  // oar << 0.1f;
  ASSERT_NO_THROW((oar << named{"primitive", 0.1f}));
}

TEST(JSONOArchive, TrivialStruct)
{
  file_handle_ostream ofs{stdout};
  json_oarchive oar{ofs};

  TrivialStruct trivial = {5, 123.f, 321.0};
  ASSERT_NO_THROW((oar << named{"trivial", trivial}));
}

TEST(JSONOArchive, TrivialNestedStruct)
{
  file_handle_ostream ofs{stdout};
  json_oarchive oar{ofs};

  TrivialNestedStruct trivial_nested = {"not", "cool", {5, 123.f, 321.0}, {99, 193.f, 1221.0}};
  ASSERT_NO_THROW((oar << named{"trivial_nested", trivial_nested}));
}

TEST(JSONOArchive, ArrayOfPrimitives)
{
  file_handle_ostream ofs{stdout};
  json_oarchive oar{ofs};

  std::vector<float> primitive_array{1.f, 2.f, 3.f, 4.f, 5.f};
  ASSERT_NO_THROW((oar << named{"array", primitive_array}));
}

TEST(JSONOArchive, ArrayOfTrivialStructs)
{
  file_handle_ostream ofs{stdout};
  json_oarchive oar{ofs};

  TrivialStruct element{5, 123.f, 321.0};
  std::vector<TrivialStruct> primitive_array{element, element, element};
  ASSERT_NO_THROW((oar << named{"array", primitive_array}));
}


TEST(JSONIArchive, Primitive)
{
  const float target = 0.1f;

  if (auto ofs_or_error = file_ostream::create("Primitive.json"); ofs_or_error.has_value())
  {
    json_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"primitive", target}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("Primitive.json"); ifs_or_error.has_value())
  {
    json_iarchive iar{*ifs_or_error};
    float read_value;
    ASSERT_NO_THROW((iar >> named{"primitive", read_value}));
    ASSERT_EQ(target, read_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}


TEST(JSONIArchive, BoolTrue)
{
  const bool target = true;

  {
    auto ofs = file_ostream::create("BoolTrue.json").value();
    json_oarchive oar{ofs};
    ASSERT_NO_THROW((oar << named{"bool", target}));
  }

  {
    auto ifs = file_istream::create("BoolTrue.json").value();
    json_iarchive iar{ifs};
    bool read_value;
    ASSERT_NO_THROW((iar >> named{"bool", read_value}));
    ASSERT_EQ(target, read_value);
  }
}


TEST(JSONIArchive, BoolFalse)
{
  const bool target = false;

  {
    auto ofs = file_ostream::create("BoolFalse.json").value();
    json_oarchive oar{ofs};
    ASSERT_NO_THROW((oar << named{"bool", target}));
  }

  {
    auto ifs = file_istream::create("BoolFalse.json").value();
    json_iarchive iar{ifs};
    bool read_value;
    ASSERT_NO_THROW((iar >> named{"bool", read_value}));
    ASSERT_EQ(target, read_value);
  }
}

TEST(JSONIArchive, TrivialStruct)
{
  const TrivialStruct target = {5, 123.f, 321.0};

  {
    auto ofs = file_ostream::create("TrivialStruct.json").value();
    json_oarchive oar{ofs};
    ASSERT_NO_THROW((oar << named{"trivial", target}));
  }

  {
    auto ifs = file_istream::create("TrivialStruct.json").value();
    json_iarchive iar{ifs};
    TrivialStruct read_value;
    ASSERT_NO_THROW((iar >> named{"trivial", read_value}));

    ASSERT_EQ(target, read_value);
  }
}


TEST(JSONIArchive, TrivialNestedStruct)
{
  const TrivialNestedStruct target = {"not", "    cool", {5, 123.f, 321.0}, {99, 193.f, 1221.0}};
  ;

  {
    auto ofs = file_ostream::create("TrivialNestedStruct.json").value();
    json_oarchive oar{ofs};
    ASSERT_NO_THROW((oar << named{"trivial_nested", target}));
  }

  {
    auto ifs = file_istream::create("TrivialNestedStruct.json").value();
    json_iarchive iar{ifs};
    TrivialNestedStruct read_value;
    ASSERT_NO_THROW((iar >> named{"trivial_nested", read_value}));

    ASSERT_EQ(target, read_value);
  }
}

TEST(JSONIArchive, ArrayOfPrimitives)
{
  const std::vector<float> target = {1.f, 2.f, 3.f, 4.f, 5.f};

  {
    auto ofs = file_ostream::create("ArrayOfPrimitives.json").value();
    json_oarchive oar{ofs};
    ASSERT_NO_THROW((oar << named{"array", target}));
  }

  {
    auto ifs = file_istream::create("ArrayOfPrimitives.json").value();
    json_iarchive iar{ifs};
    std::vector<float> read_value;
    ASSERT_NO_THROW((iar >> named{"array", read_value}));

    ASSERT_EQ(target, read_value);
  }
}

TEST(JSONIArchive, ArrayOfTrivialStructs)
{
  const TrivialStruct target_element{5, 123.f, 321.0};
  const std::vector<TrivialStruct> target = {target_element, target_element, target_element};

  {
    auto ofs = file_ostream::create("ArrayOfTrivialStructs.json").value();
    json_oarchive oar{ofs};
    ASSERT_NO_THROW((oar << named{"array", target}));
  }

  {
    auto ifs = file_istream::create("ArrayOfTrivialStructs.json").value();
    json_iarchive iar{ifs};
    std::vector<TrivialStruct> read_value;
    ASSERT_NO_THROW((iar >> named{"array", read_value}));

    ASSERT_EQ(target, read_value);
  }
}
