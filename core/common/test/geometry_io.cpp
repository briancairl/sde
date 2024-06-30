// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/serialization_binary_file.hpp"

using namespace sde;
using namespace sde::serial;


TEST(GeometryIO, Mat2f)
{
  const Mat2f target = Mat2f::Identity();

  if (auto ofs_or_error = file_ostream::create("Mat.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"mat", target}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("Mat.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    Mat2f read_value;
    ASSERT_NO_THROW((iar >> named{"mat", read_value}));

    ASSERT_TRUE(target.isApprox(read_value));
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}


TEST(GeometryIO, Bounds2f)
{
  const Bounds2f target = Bounds2f{-Vec2f::Ones(), Vec2f::Ones()};

  if (auto ofs_or_error = file_ostream::create("Bounds.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"mat", target}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("Bounds.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    Bounds2f read_value;
    ASSERT_NO_THROW((iar >> named{"mat", read_value}));

    ASSERT_TRUE(target.min().isApprox(read_value.min()));
    ASSERT_TRUE(target.max().isApprox(read_value.max()));
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}
