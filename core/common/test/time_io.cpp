// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"

using namespace sde;
using namespace sde::serial;


TEST(TimeIO, TimeOffset)
{
  const auto target_value = Seconds(5.0);

  if (auto ofs_or_error = file_ostream::create("TimeOffset.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"time_offset", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("TimeOffset.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    TimeOffset read_value;
    ASSERT_NO_THROW((iar >> named{"time_offset", read_value}));

    ASSERT_EQ(target_value, read_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}


TEST(TimeIO, Rate)
{
  const auto target_value = Hertz(5.0);

  if (auto ofs_or_error = file_ostream::create("Rate.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"rate", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("Rate.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    Rate read_value;
    ASSERT_NO_THROW((iar >> named{"rate", read_value}));

    ASSERT_EQ(target_value, read_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}
