/**
 * @copyright 2022-present Brian Cairl
 *
 * @file file_istream.hpp
 */
#pragma once

// C++ Standard Library
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>

// SDE
#include "sde/expected.hpp"
#include "sde/serial/file_stream_error.hpp"
#include "sde/serial/istream.hpp"

namespace sde::serial
{

class file_handle_istream : public istream<file_handle_istream>
{
  friend class istream<file_handle_istream>;

public:
  explicit file_handle_istream(std::FILE* file_handle);

  file_handle_istream(file_handle_istream&& other);
  file_handle_istream& operator=(file_handle_istream&& other);

  ~file_handle_istream() = default;

  void swap(file_handle_istream& other);

private:
  /**
   * @copydoc istream<file_istream>::read
   */
  std::size_t read_impl(void* ptr, std::size_t len)
  {
    const std::size_t read_bytes = std::fread(ptr, sizeof(std::byte), len, file_handle_);
    file_bytes_remaining_ -= read_bytes;
    return read_bytes;
  }

  /**
   * @copydoc istream<file_istream>::peek
   */
  char peek_impl()
  {
    char ch = std::getc(file_handle_);
    std::ungetc(ch, file_handle_);
    return ch;
  }

  /**
   * @copydoc istream<file_istream>::available
   */
  std::size_t available_impl() const { return file_bytes_remaining_; }

  /// Number of remaining bytes in file
  std::size_t file_bytes_remaining_ = 0;

protected:
  /// Native file handle
  std::FILE* file_handle_ = nullptr;
};

class file_istream final : public file_handle_istream
{
public:
  struct flags
  {
    std::uint8_t nobuf : 1;
    std::uint8_t binary : 1;
  };

  static constexpr flags default_flags{.nobuf = true, .binary = true};

  static expected<file_istream, FileStreamError>
  create(const std::filesystem::path& path, const flags fileopt = default_flags);

  file_istream(file_istream&& other) = default;
  file_istream& operator=(file_istream&& other) = default;

  ~file_istream();

private:
  explicit file_istream(std::FILE* file_handle);
};

}  // namespace sde::serial
