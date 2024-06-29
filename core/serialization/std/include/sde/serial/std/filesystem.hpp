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

template <typename OArchiveT> struct save<OArchiveT, std::filesystem::path>
{
  void operator()(OArchiveT& oar, const std::filesystem::path& path) { oar << named{"path", path.string()}; }
};

template <typename IArchiveT> struct load<IArchiveT, std::filesystem::path>
{
  void operator()(IArchiveT& iar, std::filesystem::path& path)
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
