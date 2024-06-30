// SDE
#include "sde/serial/file_istream.hpp"

namespace sde::serial
{
namespace  // anonymous
{

const char* flags_to_read_mode_str(file_istream::flags fileopt) { return fileopt.binary ? "rb" : "r"; }

}  // namespace anonymous

file_handle_istream::file_handle_istream(file_handle_istream&& other) :
    file_bytes_remaining_{other.file_bytes_remaining_}, file_handle_{other.file_handle_}
{
  other.file_handle_ = nullptr;
  other.file_bytes_remaining_ = 0;
}

file_handle_istream::file_handle_istream(std::FILE* file_handle) : file_bytes_remaining_{0}, file_handle_{file_handle}
{
  file_bytes_remaining_ = [file = file_handle_] {
    std::fseek(file, 0, SEEK_END);
    const auto size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);
    return size;
  }();
}

expected<file_istream, FileStreamError> file_istream::create(const std::filesystem::path& path, const flags fileopt)
{
  if (!std::filesystem::exists(path))
  {
    return make_unexpected(FileStreamError::kFileDoesNotExist);
  }

  std::FILE* file_handle = std::fopen(path.c_str(), flags_to_read_mode_str(fileopt));
  if (file_handle == nullptr)
  {
    return make_unexpected(FileStreamError::kFileOpenFailed);
  }

  if (fileopt.nobuf)
  {
    std::setvbuf(file_handle, nullptr, _IONBF, 0);
  }
  return file_istream{file_handle};
}

file_istream::file_istream(std::FILE* file_handle) : file_handle_istream{file_handle} {}

file_istream::~file_istream()
{
  if (file_handle_ == nullptr)
  {
    return;
  }
  std::fclose(file_handle_);
}

}  // namespace sde::serial