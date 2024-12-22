#define SDE_SCRIPT_NAME "player_character"

// C++ Standard Library
#include <array>
#include <cmath>
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/game/native_script_runtime.hpp"
#include "sde/graphics/sprite.hpp"

// RED
#include "red/components.hpp"

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

struct player_character
{
  EntityHandle entity = EntityHandle::null();
  TileSetHandle idle_frames[8UL];
  TileSetHandle walk_frames[8UL];
  TileSetHandle run_frames[8UL];
};

bool load(player_character* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  ar >> Field{"entity", self->entity};
  ar >> Field{"idle_frames", self->idle_frames};
  ar >> Field{"walk_frames", self->walk_frames};
  ar >> Field{"run_frames", self->run_frames};
  return true;
}


bool save(player_character* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  ar << Field{"entity", self->entity};
  ar << Field{"idle_frames", self->idle_frames};
  ar << Field{"walk_frames", self->walk_frames};
  ar << Field{"run_frames", self->run_frames};
  return true;
}


bool initialize(player_character* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  auto entity_or_error = resources.instance(self->entity, [](EntityCreator new_entity) {
    new_entity.attach<Size>(Size{.extent = {0.1, 0.1}});
    new_entity.attach<Position>(Position{.center = {0, 0}});
    new_entity.attach<Dynamics>(Dynamics{});
    new_entity.attach<graphics::AnimatedSprite>();
    new_entity.attach<Foreground>();
    new_entity.attach<DebugWireFrame>();
    new_entity.attach<Focused>();
  });
  if (!entity_or_error.has_value())
  {
    SDE_LOG_ERROR() << entity_or_error.error();
    return false;
  }
  else
  {
    self->entity = entity_or_error->handle;
    SDE_LOG_INFO() << self->entity << " created";
  }
  return true;
}


void edit(player_character* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return;
  }

  auto entity = resources(self->entity);
  if (!entity)
  {
    return;
  }

  ImGui::Begin(sde::format("player-%lu", self->entity.id()));
  auto [size, position, sprite] = resources.get<Registry>().get<Size, Position, graphics::AnimatedSprite>(entity->id);

  ImGui::InputFloat2("size", size.extent.data());
  ImGui::InputFloat2("position", position.center.data());

  std::array frames = {
    std::make_pair("idle", self->idle_frames),
    std::make_pair("walk", self->walk_frames),
    std::make_pair("run", self->run_frames),
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
            if (const auto h = *reinterpret_cast<const TileSetHandle*>(payload->Data); resources.exists(h))
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

bool update(player_character* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  edit(self, resources, app);

  auto entity = resources(self->entity);
  if (!entity)
  {
    return true;
  }

  auto [size, position, state, sprite] =
    resources.get<Registry>().get<Size, Position, Dynamics, graphics::AnimatedSprite>(entity->id);

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
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kBackRight] : self->run_frames[kBackRight]);
  }
  else if ((state.velocity.x() < 0) and (state.velocity.y() > 0))
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kBackLeft] : self->run_frames[kBackLeft]);
  }
  else if ((state.velocity.x() > 0) and (state.velocity.y() < 0))
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kFrontRight] : self->run_frames[kFrontRight]);
  }
  else if ((state.velocity.x() < 0) and (state.velocity.y() < 0))
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kFrontLeft] : self->run_frames[kFrontLeft]);
  }
  else if (state.velocity.x() > 0)
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kRight] : self->run_frames[kRight]);
  }
  else if (state.velocity.x() < 0)
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kLeft] : self->run_frames[kLeft]);
  }
  else if (state.velocity.y() < 0)
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kFront] : self->run_frames[kFront]);
  }
  else if (state.velocity.y() > 0)
  {
    sprite.setFrames((next_speed == kSpeedWalking) ? self->walk_frames[kBack] : self->run_frames[kBack]);
  }
  else if ((state.looking.x() > 0) and (state.looking.y() > 0))
  {
    sprite.setFrames(self->idle_frames[kBackRight]);
  }
  else if ((state.looking.x() < 0) and (state.looking.y() > 0))
  {
    sprite.setFrames(self->idle_frames[kBackLeft]);
  }
  else if ((state.looking.x() > 0) and (state.looking.y() < 0))
  {
    sprite.setFrames(self->idle_frames[kFrontRight]);
  }
  else if ((state.looking.x() < 0) and (state.looking.y() < 0))
  {
    sprite.setFrames(self->idle_frames[kBackLeft]);
  }
  else if (state.looking.x() > 0)
  {
    sprite.setFrames(self->idle_frames[kRight]);
  }
  else if (state.looking.x() < 0)
  {
    sprite.setFrames(self->idle_frames[kLeft]);
  }
  else if (state.looking.y() < 0)
  {
    sprite.setFrames(self->idle_frames[kFront]);
  }
  else if (state.looking.y() > 0)
  {
    sprite.setFrames(self->idle_frames[kBack]);
  }

  // Set sprite stuff
  if (state.velocity.x() != 0.0F or state.velocity.y() != 0.0F)
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

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(player_character);