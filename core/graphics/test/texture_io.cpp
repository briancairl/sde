// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/graphics/texture_io.hpp"
#include "sde/serialization_binary_file.hpp"

using namespace sde;
using namespace sde::serial;
using namespace sde::graphics;


TEST(TextureIO, TextureFlags)
{
  const TextureFlags target_value{
    .unpack_alignment = true,
    .generate_mip_map = false,
  };

  if (auto ofs_or_error = file_ostream::create("TextureFlags.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"texture_flags", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("TextureFlags.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    TextureFlags read_value;
    ASSERT_NO_THROW((iar >> named{"texture_flags", read_value}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}


TEST(TextureIO, TextureOptions)
{
  const TextureOptions target_value;

  if (auto ofs_or_error = file_ostream::create("TextureOptions.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"texture_options", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("TextureOptions.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    TextureOptions read_value;
    ASSERT_NO_THROW((iar >> named{"texture_options", read_value}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}
