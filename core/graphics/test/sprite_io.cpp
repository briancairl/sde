// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/resource_handle_io.hpp"
#include "sde/resource_io.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/time_io.hpp"

using namespace sde;
using namespace sde::serial;
using namespace sde::graphics;


TEST(SpriteIO, SpriteOptions)
{
  const SpriteOptions target_value;

  if (auto ofs_or_error = file_ostream::create("SpriteOptions.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"sprite_options", _R(target_value)}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("SpriteOptions.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    SpriteOptions read_value;
    ASSERT_NO_THROW((iar >> named{"sprite_options", _R(read_value)}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}

TEST(SpriteIO, AnimatedSpriteOptions)
{
  const AnimatedSpriteOptions target_value;

  if (auto ofs_or_error = file_ostream::create("AnimatedSpriteOptions.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"animated_sprite_options", _R(target_value)}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("AnimatedSpriteOptions.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    AnimatedSpriteOptions read_value;
    ASSERT_NO_THROW((iar >> named{"animated_sprite_options", _R(read_value)}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}

TEST(SpriteIO, Sprite)
{
  const Sprite target_value;

  if (auto ofs_or_error = file_ostream::create("Sprite.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"sprite", _R(target_value)}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("Sprite.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    Sprite read_value;
    ASSERT_NO_THROW((iar >> named{"sprite", _R(read_value)}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}

TEST(SpriteIO, AnimatedSprite)
{
  const AnimatedSprite target_value;

  if (auto ofs_or_error = file_ostream::create("AnimatedSprite.bin"); ofs_or_error.has_value())
  {
    binary_oarchive oar{*ofs_or_error};
    ASSERT_NO_THROW((oar << named{"animated_sprite", _R(target_value)}));
  }
  else
  {
    FAIL() << ofs_or_error.error();
  }

  if (auto ifs_or_error = file_istream::create("AnimatedSprite.bin"); ifs_or_error.has_value())
  {
    binary_iarchive iar{*ifs_or_error};
    AnimatedSprite read_value;
    ASSERT_NO_THROW((iar >> named{"animated_sprite", _R(read_value)}));

    ASSERT_EQ(read_value, target_value);
  }
  else
  {
    FAIL() << ifs_or_error.error();
  }
}