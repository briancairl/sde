// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/resource.hpp"

using namespace sde;

struct SimpleResource : Resource<SimpleResource>
{
  float a;
  int b;

  auto field_list() { return std::make_tuple(Field{"a", a}, Stub{"b", b}); }
};

TEST(Resource, Fields)
{
  SimpleResource simple;
  auto simple_tup = simple.fields();
  auto simple_tup_const = static_cast<const SimpleResource&>(simple).fields();
  ASSERT_EQ(simple_tup, simple_tup_const);
}

TEST(Resource, Values)
{
  SimpleResource simple;
  auto simple_tup = simple.values();
  auto simple_tup_const = static_cast<const SimpleResource&>(simple).values();
  ASSERT_EQ(simple_tup, simple_tup_const);
}

TEST(Resource, Names)
{
  SimpleResource simple;
  auto simple_tup = simple.names();
  EXPECT_EQ(std::get<0>(simple_tup), "a");
  EXPECT_EQ(std::get<1>(simple_tup), "b");
}

TEST(Resource, Hash)
{
  SimpleResource simple{.a = 1.F, .b = 2};
  const auto h = ResourceHasher{}(simple);
  EXPECT_EQ(h, 17077749257889412985UL);
}

struct NestedResource : Resource<NestedResource>
{
  SimpleResource a;
  int b;
  auto field_list() { return std::make_tuple(Field{"a", a}, Stub{"b", b}); }
};

namespace sde
{
template <> struct Hasher<SimpleResource> : ResourceHasher
{};

template <> struct Hasher<NestedResource> : ResourceHasher
{};
}  // namespace sde

TEST(Resource, NestedHash)
{
  NestedResource nested{.a = {.a = 1.F, .b = 2}, .b = 2};
  const auto h = ResourceHasher{}(nested);
  EXPECT_EQ(h, 6486436383776920708UL) << nested;
}

TEST(Resource, MultiHash)
{
  NestedResource nested{.a = {.a = 1.F, .b = 2}, .b = 2};
  const auto h = Hash(nested, nested);
  EXPECT_EQ(h, 17411604422488376414UL) << nested;
}