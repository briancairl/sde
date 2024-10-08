// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serial/std/vector.hpp"
#include "sde/serialization_binary_file.hpp"

using namespace sde;
using namespace sde::serial;
using namespace sde::graphics;


TEST(TileMapIO, TileMapOptions)
{
  const TileMapOptions target_value;

  if (auto ofs_or_error = file_ostream::create("TileMapOptions.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"tile_map_options", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("TileMapOptions.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    TileMapOptions read_value;
    ASSERT_NO_THROW((iar >> named{"tile_map_options", read_value}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}

TEST(TileMapIO, TileMap)
{
  const TileMap target_value{TileMapOptions{
    .tint_color = Vec4f::Ones(),
    .shape = Vec2i{100, 100},
    .tile_size = Vec2f{0.5F, 0.5F},
    .tile_set = TileSetHandle::null(),
  }};

  if (auto ofs_or_error = file_ostream::create("TileMap.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"tile_map", target_value}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("TileMap.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    TileMap read_value;
    ASSERT_NO_THROW((iar >> named{"tile_map", read_value}));
    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}
