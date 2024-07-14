/**
 * @copyright 2022-present Brian Cairl
 */

// C++ Standard Library
#include <cstring>
#include <stdexcept>
#include <utility>

// GTest
#include <gtest/gtest.h>

// SDE
#include "sde/serial/file_istream.hpp"
#include "sde/serial/file_ostream.hpp"


TEST(FileInputStream, CannotOpenFile)
{
  auto ifs_or_error = sde::serial::file_istream::create("not-a-file.bin", {.nobuf = true});
  ASSERT_FALSE(ifs_or_error.has_value());
  ASSERT_EQ(ifs_or_error.error(), sde::serial::FileStreamError::kFileDoesNotExist);
}


TEST(FileInputStream, MoveCTor)
{
  auto ifs =
    sde::serial::file_istream::create("core/serialization/stream/test/resources/file_stream.dat", {.nobuf = true})
      .value();

  ASSERT_EQ(ifs.available(), 22UL);

  sde::serial::file_istream ifs_move{std::move(ifs)};

  ASSERT_EQ(ifs.available(), 0UL);
  ASSERT_EQ(ifs_move.available(), 22UL);
}

TEST(FileInputStream, ReadAll)
{
  auto ifs =
    sde::serial::file_istream::create("core/serialization/stream/test/resources/file_stream.dat", {.nobuf = true})
      .value();

  char buf[23];
  ifs.read(buf, sizeof(buf));
  buf[sizeof(buf) - 1] = '\0';

  ASSERT_EQ(ifs.available(), 0UL);

  static const char* TARGET_VALUE = "this is just a sample\n";
  ASSERT_EQ(std::memcmp(buf, TARGET_VALUE, std::strlen(TARGET_VALUE)), 0);
}

TEST(FileInputStream, ReadTooMany)
{
  auto ifs =
    sde::serial::file_istream::create("core/serialization/stream/test/resources/file_stream.dat", {.nobuf = true})
      .value();

  char buf[23];
  ifs.read(buf, sizeof(buf) + 10);
  buf[sizeof(buf) - 1] = '\0';

  ASSERT_EQ(ifs.available(), 0UL);

  static const char* TARGET_VALUE = "this is just a sample\n";
  ASSERT_EQ(std::memcmp(buf, TARGET_VALUE, std::strlen(TARGET_VALUE)), 0);
}


TEST(FileOutputStream, CreateFileOnAppend)
{
  auto ofs_or_error = sde::serial::file_ostream::create("ostream-append-not-a-file.bin", {.append = true});
  ASSERT_TRUE(ofs_or_error.has_value()) << ofs_or_error.error();
}


TEST(FileOutputStream, CreateFileOnWrite)
{
  auto ofs_or_error = sde::serial::file_ostream::create("ostream-write-not-a-file.bin", {.append = false});
  ASSERT_TRUE(ofs_or_error.has_value()) << ofs_or_error.error();
}


TEST(FileOutputStream, Write)
{
  char buf[] = "this is a sample payload for write";
  auto ofs = sde::serial::file_ostream::create("write.bin").value();
  ASSERT_EQ(sizeof(buf), ofs.write(buf));
}

TEST(FileStream, WriteThenRead)
{
  char write_buf[] = "this is a sample payload for readback";
  auto ofs = sde::serial::file_ostream::create("readback.bin").value();
  ASSERT_EQ(sizeof(write_buf), ofs.write(write_buf));

  char read_buf[sizeof(write_buf) * 2];
  auto ifs = sde::serial::file_istream::create("readback.bin").value();
  ASSERT_EQ(ifs.read(read_buf), sizeof(write_buf));

  ASSERT_EQ(std::memcmp(write_buf, read_buf, sizeof(write_buf)), 0);
}
