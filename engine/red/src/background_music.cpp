// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/render_buffer.hpp"
#include "sde/graphics/renderer.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/type_set.hpp"
#include "sde/graphics/type_setter.hpp"
#include "sde/logging.hpp"

// RED
#include "red/components.hpp"
#include "red/renderer.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::audio;

class BackgroundMusic final : public ScriptRuntime
{
public:
  BackgroundMusic() : ScriptRuntime{"BackgroundMusic"} {}

private:
  SoundHandle music_;
  SoundHandle ambiance_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> Field{"music", music_};
    ar >> Field{"ambiance", ambiance_};
    return true;
  }

  bool onSave(OArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar << Field{"music", music_};
    ar << Field{"ambiance", ambiance_};
    return true;
  }

  bool onInitialize(Systems& systems, SharedAssets& assets, const AppProperties& app) override
  {
    if (!assets.assign(music_, "/home/brian/dev/assets/sounds/tracks/OldTempleLoop.wav"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!assets.assign(ambiance_, "/home/brian/dev/assets/sounds/tracks/forest.wav"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (auto listener_or_err = ListenerTarget::create(systems.mixer, 0UL); listener_or_err.has_value())
    {
      if (auto sound = assets.audio.sounds(music_))
      {
        listener_or_err->set(sound.get(), TrackOptions{.volume = 0.2F, .looped = true});
      }
      if (auto sound = assets.audio.sounds(ambiance_))
      {
        listener_or_err->set(sound.get(), TrackOptions{.volume = 0.4F, .looped = true});
      }
    }
    return true;
  }

  expected<void, ScriptError> onUpdate(Systems& systems, SharedAssets& assets, const AppProperties& app) override
  {
    return {};
  }
};

std::unique_ptr<sde::game::ScriptRuntime> createBackgroundMusic() { return std::make_unique<BackgroundMusic>(); }
