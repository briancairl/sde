// C++ Standard Library
#include <ostream>

// SDE
#include "sde/audio/sound_channel_format.hpp"

namespace sde::audio
{

std::ostream& operator<<(std::ostream& os, SoundChannelCount count)
{
  switch (count)
  {
  // clang-format off
  case SoundChannelCount::kMono: return os << "Mono";
  case SoundChannelCount::kStereo: return os << "Stereo";
    // clang-format on
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, SoundChannelBitDepth element_type)
{
  switch (element_type)
  {
  // clang-format off
  case SoundChannelBitDepth::kU8: return os << "U8";
  case SoundChannelBitDepth::kU16: return os << "U16";
    // clang-format on
  }
  return os;
}

}  // namespace sde::audio
