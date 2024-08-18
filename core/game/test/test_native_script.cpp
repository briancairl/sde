// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/game/library.hpp"
#include "sde/game/native_script.hpp"

using namespace sde::game;

TEST(NativeScriptCache, InvalidScriptLibrary)
{
  LibraryCache libraries;
  NativeScriptCache cache{libraries};

  const auto lib_or_error = cache.create(sde::asset::path{"no"});
  ASSERT_FALSE(lib_or_error.has_value());
}

TEST(NativeScriptCache, ValidScriptLibrary)
{
  LibraryCache libraries;
  NativeScriptCache cache{libraries};

  const auto lib_or_error =
    cache.create(sde::asset::path{"_solib_k8/libcore_Sgame_Stest_Slibscript_Ulibrary_Utest.so"});
  ASSERT_TRUE(lib_or_error.has_value());
}
