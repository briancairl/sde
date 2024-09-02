// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/game/library.hpp"

using namespace sde::game;

TEST(LibraryCache, InvalidLibrary)
{
  LibraryCache cache;

  const auto lib_or_error = cache.create(sde::asset::path{"no"});
  ASSERT_FALSE(lib_or_error.has_value());
}

TEST(LibraryCache, ValidLibrary)
{
  LibraryCache cache;

  const auto lib_or_error =
    cache.create(sde::asset::path{"_solib_k8/libcore_Sgame_Stest_Slibscript_Ulibrary_Utest.so"});
  ASSERT_TRUE(lib_or_error.has_value());

  const auto symbol_or_error = lib_or_error->value->lib.get("on_create");
  ASSERT_TRUE(symbol_or_error.has_value()) << symbol_or_error.error().details;
}
