// C++ Standard Library
#include <array>
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/script_impl.hpp"
#include "sde/game/systems.hpp"
#include "sde/geometry_io.hpp"
#include "sde/graphics/sprite.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"
#include "sde/logging.hpp"
#include "sde/serial/std/string.hpp"
#include "sde/time_io.hpp"

// RED
#include "red/components.hpp"
#include "red/player_character.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

namespace
{

constexpr std::size_t kFront{0};
constexpr std::size_t kBack{1};
constexpr std::size_t kRight{2};
constexpr std::size_t kLeft{3};
constexpr std::size_t kFrontRight{4};
constexpr std::size_t kFrontLeft{5};
constexpr std::size_t kBackRight{6};
constexpr std::size_t kBackLeft{7};

constexpr std::array kDirectionLabels = {
  "Front",
  "Back",
  "Right",
  "Left",
  "FrontRight",
  "FrontLeft",
  "BackRight",
  "BackLeft",
};

}  // namespace

class PlayerCharacter final : public ScriptRuntime
{
public:
  PlayerCharacter() : ScriptRuntime{"PlayerCharacter"} {}

private:
  EntityHandle entity_;
  TileSetHandle idle_frames_[8UL];
  TileSetHandle walk_frames_[8UL];
  TileSetHandle run_frames_[8UL];

  bool onLoad(IArchive& ar) override
  {
    using namespace sde::serial;
    ar >> Field{"entity", entity_};
    ar >> Field{"idle_frames", idle_frames_};
    ar >> Field{"walk_frames", walk_frames_};
    ar >> Field{"run_frames", run_frames_};
    return true;
  }

  bool onSave(OArchive& ar) const override
  {
    using namespace sde::serial;
    ar << Field{"entity", entity_};
    ar << Field{"idle_frames", idle_frames_};
    ar << Field{"walk_frames", walk_frames_};
    ar << Field{"run_frames", run_frames_};
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!entity_.isNull())
    {
      return true;
    }
    auto entity_or_error = assets.entities.make_entity([](EntityCache& cache, Entity& e) {
      cache.attach<Size>(e, Size{.extent = {0.1, 0.1}});
      cache.attach<Position>(e, Position{.center = {0, 0}});
      cache.attach<Dynamics>(e, Dynamics{});
      cache.attach<graphics::AnimatedSprite>(e);
      cache.attach<Foreground>(e);
    });
    if (!entity_or_error.has_value())
    {
      SDE_LOG_ERROR("failed");
      return false;
    }
    entity_ = entity_or_error->handle;
    return true;
  }

  void onEdit(SharedAssets& assets, AppState& app_state, const AppProperties& app)
  {
    if (!assets->contains<ImGuiContext*>())
    {
      return;
    }
    ImGui::Begin("player");

    auto entity = assets.entities.get_if(entity_);
    auto [size, position, sprite] = assets.registry.get<Size, Position, graphics::AnimatedSprite>(entity->id);

    ImGui::InputFloat2("size", size.extent.data());

    std::array frames = {
      std::make_pair("idle", idle_frames_),
      std::make_pair("walk", walk_frames_),
      std::make_pair("run", run_frames_),
    };

    for (auto& [frame_label, frames] : frames)
    {
      if (ImGui::CollapsingHeader(frame_label))
      {
        auto direction_label_itr = std::begin(kDirectionLabels);
        for (auto frames_itr = frames; frames_itr != (frames + 8UL); ++frames_itr, ++direction_label_itr)
        {
          ImGui::Dummy(ImVec2{10, 10});
          if (*frames_itr == sprite.options().frames)
          {
            ImGui::TextColored(ImVec4{0, 1, 0, 1}, "%s: %lu", *direction_label_itr, frames_itr->id());
          }
          else if (frames_itr->isNull())
          {
            ImGui::TextColored(ImVec4{1, 0, 0, 1}, "%s: %lu", *direction_label_itr, frames_itr->id());
          }
          else
          {
            ImGui::TextColored(ImVec4{1, 1, 1, 1}, "%s: %lu", *direction_label_itr, frames_itr->id());
          }
          if (ImGui::BeginDragDropTarget())
          {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_TILESET_ASSET"))
            {
              SDE_ASSERT_EQ(payload->DataSize, sizeof(TileSetHandle));
              if (const auto h = *reinterpret_cast<const TileSetHandle*>(payload->Data);
                  assets.graphics.tile_sets.exists(h))
              {
                (*frames_itr) = h;
              }
            }
            ImGui::EndDragDropTarget();
          }
        }
      }
    }
    ImGui::End();
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    onEdit(assets, app_state, app);

    if (!app_state.enabled)
    {
      return {};
    }

    auto entity = assets.entities.get_if(entity_);
    auto [size, position, state, sprite] =
      assets.registry.get<Size, Position, Dynamics, graphics::AnimatedSprite>(entity->id);

    static constexpr float kSpeedWalking = 0.5;
    static constexpr float kSpeedRunning = 1.0;

    // Handle character speed
    const float next_speed = app.keys.isDown(KeyCode::kLShift) ? kSpeedRunning : kSpeedWalking;

    state.velocity.setZero();
    sprite.setMode(graphics::AnimatedSprite::Mode::kLooped);

    // Handle movement controls
    if (app.keys.isDown(KeyCode::kA))
    {
      state.velocity.x() = -next_speed;
    }
    if (app.keys.isDown(KeyCode::kD))
    {
      state.velocity.x() = +next_speed;
    }
    if (app.keys.isDown(KeyCode::kS))
    {
      state.velocity.y() = -next_speed;
    }
    if (app.keys.isDown(KeyCode::kW))
    {
      state.velocity.y() = +next_speed;
    }

    TileSetHandle frames;

    // Handle next animation
    if ((state.velocity.x() > 0) and (state.velocity.y() > 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kBackRight] : run_frames_[kBackRight]);
    }
    else if ((state.velocity.x() < 0) and (state.velocity.y() > 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kBackLeft] : run_frames_[kBackLeft]);
    }
    else if ((state.velocity.x() > 0) and (state.velocity.y() < 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kFrontRight] : run_frames_[kFrontRight]);
    }
    else if ((state.velocity.x() < 0) and (state.velocity.y() < 0))
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kFrontLeft] : run_frames_[kFrontLeft]);
    }
    else if (state.velocity.x() > 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kRight] : run_frames_[kRight]);
    }
    else if (state.velocity.x() < 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kLeft] : run_frames_[kLeft]);
    }
    else if (state.velocity.y() < 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kFront] : run_frames_[kFront]);
    }
    else if (state.velocity.y() > 0)
    {
      sprite.setFrames((next_speed == kSpeedWalking) ? walk_frames_[kBack] : run_frames_[kBack]);
    }
    else if ((state.looking.x() > 0) and (state.looking.y() > 0))
    {
      sprite.setFrames(idle_frames_[kBackRight]);
    }
    else if ((state.looking.x() < 0) and (state.looking.y() > 0))
    {
      sprite.setFrames(idle_frames_[kBackLeft]);
    }
    else if ((state.looking.x() > 0) and (state.looking.y() < 0))
    {
      sprite.setFrames(idle_frames_[kFrontRight]);
    }
    else if ((state.looking.x() < 0) and (state.looking.y() < 0))
    {
      sprite.setFrames(idle_frames_[kBackLeft]);
    }
    else if (state.looking.x() > 0)
    {
      sprite.setFrames(idle_frames_[kRight]);
    }
    else if (state.looking.x() < 0)
    {
      sprite.setFrames(idle_frames_[kLeft]);
    }
    else if (state.looking.y() < 0)
    {
      sprite.setFrames(idle_frames_[kFront]);
    }
    else if (state.looking.y() > 0)
    {
      sprite.setFrames(idle_frames_[kBack]);
    }

    // Set sprite stuff
    if ((state.velocity.array() != 0.0F).any())
    {
      state.looking = state.velocity;
      sprite.setFrameRate(Hertz(next_speed * 15.0F));
    }
    else
    {
      sprite.setFrameRate(Hertz(kSpeedWalking * 15.0F));
    }

    // if (auto listener_or_err = audio::ListenerTarget::create(systems.mixer, kPlayerListener);
    //     listener_or_err.has_value())
    // {
    //   listener_or_err->set(audio::ListenerState{
    //     .position = Vec3f{position.center.x(), position.center.y(), 1.0F},
    //     .velocity = Vec3f{state.velocity.x(), state.velocity.y(), 0.0F},
    //     .orientation_at = Vec3f::UnitY(),
    //     .orientation_up = Vec3f::UnitZ(),
    //   });
    // }

    return {};
  }
};

std::unique_ptr<ScriptRuntime> _PlayerCharacter() { return std::make_unique<PlayerCharacter>(); }