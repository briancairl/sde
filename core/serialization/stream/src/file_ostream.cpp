// SDE
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

}  // namespace anonymous

expected<file_ostream, FileStreamError> file_ostream::create(const std::filesystem::path& path, const flags fileopt)
{
  std::FILE* file_handle = std::fopen(path.c_str(), flags_to_write_mode_str(fileopt));
  if (file_handle == nullptr)
  {
    return make_unexpected(FileStreamError::kFileOpenFailed);
  }

  if (fileopt.nobuf)
  {
    std::setvbuf(file_handle, nullptr, _IONBF, 0);
  }
  return file_ostream{file_handle};
}

file_ostream::file_ostream(std::FILE* file_handle) : file_handle_ostream{file_handle} {}

file_ostream::~file_ostream()
{
  if (file_handle_ == nullptr)
  {
    return;
  }
  std::fclose(file_handle_);
}

}  // namespace sde::serial