// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/game/component.hpp"
#include "sde/game/library.hpp"

using namespace sde::game;

TEST(ComponentCache, InvalidScriptLibrary)
{
  LibraryCache libraries;
  ComponentCache cache{libraries};

  const auto lib_or_error = cache.create(sde::asset::path{"no"});
  ASSERT_FALSE(lib_or_error.has_value());
}

TEST(ComponentCache, ValidScriptLibrary)
{
  LibraryCache libraries;
  ComponentCache cache{libraries};

  const auto lib_or_error = cache.create(sde::asset::path{"_solib_k8/libcore_Sgame_Stest_Slibcomponent_Utest.so"});
  ASSERT_TRUE(lib_or_error.has_value());

  const auto* component_data = cache.get_if("TestComponent");
  ASSERT_NE(component_data, nullptr);

  EXPECT_EQ(component_data->name, "TestComponent") << component_data->name;
}
