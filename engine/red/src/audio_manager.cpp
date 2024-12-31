#define SDE_SCRIPT_TYPE_NAME "audio_manager"

// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/audio/mixer.hpp"
#include "sde/game/native_script_runtime.hpp"

// RED
#include "red/components/audio.hpp"
#include "red/components/common.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::audio;

struct SoundHandleHash
{
  std::size_t operator()(const SoundHandle& handle) const { return handle.id(); }
};

struct audio_manager : native_script_data
{
  std::optional<Mixer> mixer;
  std::unordered_map<SoundHandle, TrackPlayback, SoundHandleHash> sound_playing;
};

template <typename ArchiveT> bool serialize(audio_manager* self, ArchiveT& ar) { return true; }

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

bool shutdown(audio_manager* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

void ui(audio_manager* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return;
  }
  ImGui::Begin(self->guid());
  for (const auto& [handle, sound] : resources.get<audio::SoundCache>())
  {
    auto sound_data = resources(sound->sound_data);

    SDE_ASSERT_TRUE(sound_data);

    ImGui::PushID(handle.id());
    ImGui::Text("Sound[%lu] : %s", handle.id(), sound_data->path.c_str());

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
      const ImVec4 tint =
        ImGui::SetDragDropPayload("SDE_SOUND_ASSET", std::addressof(handle), sizeof(handle), /*cond = */ 0)
        ? ImVec4{0, 1, 0, 1}
        : ImVec4{1, 1, 1, 1};
      ImGui::TextColored(tint, "Sound[%lu] : %s", handle.id(), sound_data->path.c_str());
      ImGui::EndDragDropSource();
    }

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
}

bool update(audio_manager* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  ui(self, resources, app);

  auto& reg = resources.get<Registry>();
  if (auto target_or_error = ListenerTarget::create(*self->mixer, kPlayerListener))
  {
    reg.view<Position, Dynamics, SFXPlayback, Focused>().each(
      [&target = *target_or_error, &reg](const Position& position, const Dynamics& dynamics, SFXPlayback& playback) {
        target.set(ListenerState{
          .gain = 1.0F,
          .position = Vec3f{position.center.x(), position.center.y(), 1.0F},
          .velocity = Vec3f{dynamics.velocity.x(), dynamics.velocity.y(), 0.0F},
          .orientation_at = Vec3f::UnitY(),
          .orientation_up = Vec3f::UnitZ()});
      });

    reg.view<Position, SFXPlayback>().each(
      [&target = *target_or_error, &reg, &resources](const Position& position, SFXPlayback& playback) {
        if (!playback.state.has_value())
        {
          if (playback.repeat == 0)
          {
            return;
          }
          else
          {
            --playback.repeat;
          }

          if (auto sound = resources(playback.sound); !sound)
          {
            return;
          }
          else if (auto playback_or_error = target.set(sound.get()))
          {
            playback_or_error->setVolume(playback.volume);
            playback_or_error->setLooped(playback.looped);
            playback.state.emplace(std::move(playback_or_error).value());
          }
          else
          {
            SDE_LOG_WARN() << playback_or_error.error();
          }
        }
        else if (!playback.state->isValid() or playback.state->track()->stopped())
        {
          playback.reset();
        }
      });
  }

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(audio_manager);