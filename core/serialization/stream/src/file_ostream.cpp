/**
 * @copyright 2022-present Brian Cairl
 *
 * @file file_ostream.cpp
 */

// C++ Standard Library
#include <stdexcept>

// SDE
#include "sde/format.hpp"
#include "sde/serial/file_ostream.hpp"

namespace sde::serial
{
namespace  // anonymous
{

const char* flags_to_write_mode_str(file_ostream::flags fileopt)
{
  if (fileopt.append)
  {
    return fileopt.binary ? "ab" : "a";
  }
  else
  {
    return fileopt.binary ? "wb" : "w";
  }
}

const char* flags_to_write_mode_str_human_readable(file_ostream::flags fileopt)
{
  if (fileopt.append)
  {
    return fileopt.binary ? "append|binary" : "append";
  }
  else
  {
    return fileopt.binary ? "write|binary" : "write";
  }
}

}  // namespace anonymous

file_ostream::file_ostream(const char* filename, const flags fileopt) :
    file_handle_ostream{std::fopen(filename, flags_to_write_mode_str(fileopt))}
{
  if (file_handle_ == nullptr)
  {
    throw std::runtime_error{
      format<32UL>("failed to to open file (%s) for %s", filename, flags_to_write_mode_str_human_readable(fileopt))};
  }

  if (fileopt.nobuf)
  {
    std::setvbuf(file_handle_, nullptr, _IONBF, 0);
  }
}

file_ostream::~file_ostream()
{
  if (file_handle_ == nullptr)
  {
    return;
  }
  std::fclose(file_handle_);
}

}  // namespace sde::serial