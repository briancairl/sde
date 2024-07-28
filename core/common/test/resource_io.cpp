// C++ Standard Library
#include <filesystem>
#include <string>
#include <vector>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/resource.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/filesystem.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"

using namespace sde;
using namespace sde::serial;

namespace std
{
template <typename T> std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec)
{
  os << '[';
  for (const auto& e : vec)
  {
    os << e << ' ';
  }
  return os << ']';
}
}  // namespace std

struct SimpleResource : Resource<SimpleResource>
{
  std::vector<float> a;
  std::string b;

  auto field_list() { return FieldList(Field{"a", a}, Field{"b", b}); }
};

struct NestedResource : Resource<NestedResource>
{
  SimpleResource a;
  std::filesystem::path b;
  auto field_list() { return FieldList(Field{"a", a}, Field{"b", b}); }
};

TEST(ResourceIO, SimpleResource)
{
  const SimpleResource simple{.a = {1, 2, 3, 4}, .b = "ok"};

  if (auto ofs_or_error = file_ostream::create("SimpleResource.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"simple", simple}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }


  if (auto ifs_or_error = file_istream::create("SimpleResource.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    SimpleResource read_value;

    ASSERT_NO_THROW((iar >> named{"simple", read_value}));

    std::cerr << simple << std::endl;

    std::cerr << read_value << std::endl;
    ASSERT_EQ(simple, read_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}

TEST(ResourceIO, NestedResource)
{
  const NestedResource nested{.a = {.a = {1, 2, 3, 4}, .b = "ok"}, .b = "nok"};

  if (auto ofs_or_error = file_ostream::create("NestedResource.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"nested", nested}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }


  if (auto ifs_or_error = file_istream::create("NestedResource.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    NestedResource read_value;

    ASSERT_NO_THROW((iar >> named{"nested", read_value}));

    std::cerr << nested << std::endl;

    std::cerr << read_value << std::endl;
    ASSERT_EQ(nested, read_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}
