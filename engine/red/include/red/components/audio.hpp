#pragma once

// C++ Standard Library
#include <optional>

// SDE
#include "sde/audio/sound.hpp"
#include "sde/audio/track.hpp"
#include "sde/game/component_decl.hpp"
#include "sde/geometry.hpp"
#include "sde/resource.hpp"

using namespace sde;
using namespace sde::audio;

struct SFXPlayback : Resource<SFXPlayback>
{
  SoundHandle sound = SoundHandle::null();
  std::optional<TrackPlayback> state = {};
  bool looped = false;
  float volume = 1.0F;
  std::size_t repeat = 0;

  void reset()
  {
    sound = SoundHandle::null();
    state.reset();
  }

  void setSound(SoundHandle next_sound, std::size_t repeat_count = 1)
  {
    if (this->sound == next_sound)
    {
      return;
    }
    else
    {
      this->state.reset();
      this->sound = next_sound;
      this->looped = (repeat_count == 0);
      this->repeat = (repeat_count == 0) ? 1 : repeat_count;
    }
  }

  auto field_list() { return FieldList(Field{"sound", sound}, Field{"volume", volume}, Field{"repeat", repeat}); }
};

static constexpr std::size_t kGlobalListener = 0;
static constexpr std::size_t kPlayerListener = 1;
