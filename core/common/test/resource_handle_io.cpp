// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/resource_handle_io.hpp"
#include "sde/serialization_binary_file.hpp"

using namespace sde;
using namespace sde::serial;


TEST(ResourceHandleIO, ResourceHandle)
{
  const ResourceHandle<std::size_t> target_value{1234};

  if (auto ofs_or_error = file_ostream::create("Mat.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"mat", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("Mat.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    ResourceHandle<std::size_t> read_value;
    ASSERT_NO_THROW((iar >> named{"mat", read_value}));

    ASSERT_EQ(target_value, read_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}
