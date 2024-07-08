// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/resource.hpp"

using namespace sde;

struct SimpleResource : Resource<SimpleResource>
{
  float a;
  int b;

  auto fields_list() { return std::make_tuple(Field{"a", a}, Field{"b", b}); }
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
