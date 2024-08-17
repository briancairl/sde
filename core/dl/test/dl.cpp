// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/dl/library.hpp"

using namespace sde;

TEST(DL, InvalidLibrary)
{
  const auto lib_or_error = dl::Library::load("no");
  ASSERT_FALSE(lib_or_error.has_value());
}

TEST(DL, ValidLibrary)
{
  const auto lib_or_error = dl::Library::load("_solib_k8/libcore_Sdl_Stest_Sliblibtest.so");
  ASSERT_TRUE(lib_or_error.has_value()) << lib_or_error.error().details;
}

TEST(DL, SymbolLookup)
{
  const auto lib_or_error = dl::Library::load("_solib_k8/libcore_Sdl_Stest_Sliblibtest.so");
  ASSERT_TRUE(lib_or_error.has_value()) << lib_or_error.error().details;

  {
    const auto sym_or_error = lib_or_error->get("_Z4funcPKci1111");
    ASSERT_FALSE(sym_or_error.has_value());
  }

  {
    const auto sym_or_error = lib_or_error->get("_Z4funcPKci");
    ASSERT_TRUE(sym_or_error.has_value()) << sym_or_error.error().details;
  }
}

TEST(DL, Function)
{
  const auto lib_or_error = dl::Library::load("_solib_k8/libcore_Sdl_Stest_Sliblibtest.so");
  ASSERT_TRUE(lib_or_error.has_value()) << lib_or_error.error().details;

  const auto sym_or_error = lib_or_error->get("_Z4funcPKci");
  ASSERT_TRUE(sym_or_error.has_value()) << sym_or_error.error().details;

  dl::Function<void(const char*, int)> dl_fn{*sym_or_error};

  dl_fn("this", 1);
}
