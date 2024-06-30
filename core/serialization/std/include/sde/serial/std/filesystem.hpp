/**
 * @copyright 2023-present Brian Cairl
 *
 * @file filesystem.hpp
 */
#pragma once

// C++ Standard Library
#include <filesystem>

// SDE
#include "sde/serial/object.hpp"
#include "sde/serial/packet.hpp"
#include "sde/serial/std/string.hpp"

namespace sde::serial
{

template <typename ArchiveT> struct save<ArchiveT, std::filesystem::path>
{
  void operator()(ArchiveT& oar, const std::filesystem::path& path) { oar << named{"path", path.string()}; }
};

template <typename ArchiveT> struct load<ArchiveT, std::filesystem::path>
{
  void operator()(ArchiveT& iar, std::filesystem::path& path)
  {
    std::string path_str;
    iar >> named{"path", path_str};
    path = std::filesystem::path{std::move(path_str)};
  }
};

template <typename ArchiveT> struct serialize<ArchiveT, std::filesystem::file_type>
{
  void operator()(ArchiveT& ar, std::filesystem::file_type& file_type)
  {
    ar& named{"file_type", make_packet(std::addressof(file_type))};
  }
};

}  // namespace sde::serial
