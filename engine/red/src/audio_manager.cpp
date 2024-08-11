// C++ Standard Library
#include <optional>
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// RED
#include "red/audio_manager.hpp"
#include "red/imgui_common.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::audio;


class AudioManager final : public ScriptRuntime
{
public:
  AudioManager() :
      ScriptRuntime{"AudioManager"},
      mixer_{

      }
  {}

private:
  std::optional<Mixer> mixer_;

  struct SoundHandleHash
  {
    std::size_t operator()(const SoundHandle& handle) const { return handle.id(); }
  };

  std::unordered_map<SoundHandle, TrackPlayback, SoundHandleHash> playing_;

  bool onLoad(IArchive& ar) override
  {
    using namespace sde::serial;
    return true;
  }

  bool onSave(OArchive& ar) const override
  {
    using namespace sde::serial;
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    auto mixer_or_error = Mixer::create();
    if (!mixer_or_error.has_value())
    {
      SDE_LOG_ERROR("Failed to create mixer");
      return false;
    }
    mixer_.emplace(std::move(mixer_or_error).value());
    SDE_LOG_DEBUG("Created mixer");
    return true;
  }

  void onEdit(SharedAssets& assets, AppState& app_state, const AppProperties& app)
  {
    if (!assets->contains<ImGuiContext*>())
    {
      return;
    }

    ImGui::Begin("sounds");
    for (const auto& [handle, sound] : assets.audio.sounds)
    {
      ImGui::PushID(handle.id());
      ImGui::Text("sound[%lu]", handle.id());
      if (auto itr = playing_.find(handle); itr != std::end(playing_))
      {
        if (ImGui::Button("stop") or !itr->second.isValid())
        {
          itr->second.stop();
          playing_.erase(itr);
        }
        else if (auto track = itr->second.track(); track->stopped())
        {
          playing_.erase(itr);
        }
        else if (float p = track->progress(); ImGui::SameLine(), ImGui::SliderFloat("##progress", &p, 0, 1))
        {
          track->jump(p);
        }
      }
      else if (ImGui::ArrowButton("play", ImGuiDir_Right))
      {
        if (auto target_or_error = ListenerTarget::create(*mixer_, 0))
        {
          if (auto playback_or_error = target_or_error->set(sound.value))
          {
            playing_.emplace(handle, std::move(playback_or_error).value());
          }
        }
      }
      ImGui::PopID();
    }
    ImGui::End();
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    onEdit(assets, app_state, app);
    return {};
  }
};

std::unique_ptr<ScriptRuntime> _AudioManager() { return std::make_unique<AudioManager>(); }
