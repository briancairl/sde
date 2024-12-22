#define SDE_SCRIPT_NAME "audio_manager"

// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/audio/mixer.hpp"
#include "sde/game/native_script_runtime.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::audio;

struct SoundHandleHash
{
  std::size_t operator()(const SoundHandle& handle) const { return handle.id(); }
};

struct audio_manager
{
  std::optional<Mixer> mixer;
  std::unordered_map<SoundHandle, TrackPlayback, SoundHandleHash> sound_playing;
};

bool load(audio_manager* self, sde::game::IArchive& ar) { return true; }

bool save(audio_manager* self, sde::game::OArchive& ar) { return true; }

bool initialize(audio_manager* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  auto mixer_or_error = Mixer::create(app.sound_device);

  if (!mixer_or_error.has_value())
  {
    SDE_LOG_ERROR() << mixer_or_error.error();
    return false;
  }

  {
    SDE_LOG_DEBUG() << "Created mixer";
    self->mixer.emplace(std::move(mixer_or_error).value());
    return true;
  }
}

bool update(audio_manager* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }
  ImGui::Begin("sounds");
  for (const auto& [handle, sound] : resources.get<audio::SoundCache>())
  {
    ImGui::PushID(handle.id());
    ImGui::Text("sound[%lu]", handle.id());
    if (auto itr = self->sound_playing.find(handle); itr != std::end(self->sound_playing))
    {
      if (ImGui::Button("stop") or !itr->second.isValid())
      {
        itr->second.stop();
        self->sound_playing.erase(itr);
      }
      else if (auto track = itr->second.track(); track->stopped())
      {
        self->sound_playing.erase(itr);
      }
      else if (float p = track->progress(); ImGui::SameLine(), ImGui::SliderFloat("##progress", &p, 0, 1))
      {
        track->jump(p);
      }
    }
    else if (ImGui::ArrowButton("play", ImGuiDir_Right))
    {
      if (auto target_or_error = ListenerTarget::create(*self->mixer, 0))
      {
        if (auto playback_or_error = target_or_error->set(sound.value))
        {
          self->sound_playing.emplace(handle, std::move(playback_or_error).value());
        }
      }
    }
    ImGui::PopID();
  }
  ImGui::End();
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(audio_manager);