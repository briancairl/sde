// C++ Standard Library
#include <array>
#include <cmath>
#include <ostream>

// EnTT
#include <entt/entt.hpp>

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

}  // namespace

class Components : public Resource<Components>
{
  friend fundemental_type;

public:
  explicit Components(entt::entity id, entt::registry& reg) : id_{id}, registry_{&reg}
  {
    registry_->emplace<Focused>(id_);
    registry_->emplace<Midground>(id_);
    registry_->emplace<Info>(id_, Info{.name = {"bob"}});
    registry_->emplace<Size>(id_, Size{.extent = {1.5F, 1.5F}});
    registry_->emplace<Position>(id_, Position{.center = Vec2f::Zero()});
    registry_->emplace<Dynamics>(id_, Dynamics{.velocity = Vec2f::Zero(), .looking = {0, -1}});
    registry_->emplace<AnimatedSprite>(id_).setMode(AnimatedSprite::Mode::kLooped);
  }

private:
  entt::entity id_;
  entt::registry* registry_;

  auto field_list()
  {
    return FieldList(
      Field{"info", registry_->get<Info>(id_)},
      Field{"size", registry_->get<Size>(id_)},
      Field{"position", registry_->get<Position>(id_)},
      Field{"dynamics", registry_->get<Dynamics>(id_)},
      Field{"sprite", registry_->get<AnimatedSprite>(id_)});
  }
};

class PlayerCharacter final : public ScriptRuntime
{
public:
  PlayerCharacter() : ScriptRuntime{"PlayerCharacter"} {}

private:
  bool AssignMovementTileSets(
    game::Assets& assets,
    TileSetHandle* movement_tilsets,
    std::size_t cardinal_start_offset,
    std::size_t ordinal_start_offset) const;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> Field{"entity", entity_};
    ar >> Field{"front_atlas", front_atlas_};
    ar >> Field{"back_atlas", back_atlas_};
    ar >> Field{"side_atlas", side_atlas_};
    ar >> Field{"front_side_atlas", front_side_atlas_};
    ar >> Field{"back_side_atlas", back_side_atlas_};
    ar >> Field{"idle_frames", idle_frames_};
    ar >> Field{"walk_frames", walk_frames_};
    ar >> Field{"run_frames", run_frames_};

    if (auto entity = assets.entities.get_if(entity_); entity == nullptr)
    {
      return false;
    }
    else
    {
      components_.emplace(entity->id, assets.registry);
      ar >> Field{"components", *components_};
    }
    return true;
  }

  bool onSave(OArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar << Field{"entity", entity_};
    ar << Field{"front_atlas", front_atlas_};
    ar << Field{"back_atlas", back_atlas_};
    ar << Field{"side_atlas", side_atlas_};
    ar << Field{"front_side_atlas", front_side_atlas_};
    ar << Field{"back_side_atlas", back_side_atlas_};
    ar << Field{"idle_frames", idle_frames_};
    ar << Field{"walk_frames", walk_frames_};
    ar << Field{"run_frames", run_frames_};
    ar << Field{"components", *components_};
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets.assign(front_atlas_, "/home/brian/dev/assets/sprites/red/Top Down/Front Movement.png"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!assets.assign(back_atlas_, "/home/brian/dev/assets/sprites/red/Top Down/Back Movement.png"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!assets.assign(side_atlas_, "/home/brian/dev/assets/sprites/red/Top Down/Side Movement.png"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!assets.assign(front_side_atlas_, "/home/brian/dev/assets/sprites/red/Top Down/FrontSide Movement.png"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!assets.assign(back_side_atlas_, "/home/brian/dev/assets/sprites/red/Top Down/BackSide Movement.png"_path))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!AssignMovementTileSets(assets, idle_frames_, 18UL, 12UL))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!AssignMovementTileSets(assets, walk_frames_, 12UL, 12UL))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (!AssignMovementTileSets(assets, run_frames_, 6UL, 6UL))
    {
      SDE_LOG_ERROR("failed");
      return false;
    }

    if (auto status_or_error = assets.assign(entity_); !status_or_error)
    {
      return false;
    }
    else if (*status_or_error == ResourceStatus::kCreated)
    {
      auto entity = assets.entities.get_if(entity_);
      components_.emplace(entity->id, assets.registry);
    }
    return true;
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
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

    if (auto listener_or_err = audio::ListenerTarget::create(systems.mixer, kPlayerListener);
        listener_or_err.has_value())
    {
      listener_or_err->set(audio::ListenerState{
        .position = Vec3f{position.center.x(), position.center.y(), 1.0F},
        .velocity = Vec3f{state.velocity.x(), state.velocity.y(), 0.0F},
        .orientation_at = Vec3f::UnitY(),
        .orientation_up = Vec3f::UnitZ(),
      });
    }

    return {};
  }

  EntityHandle entity_;
  TextureHandle front_atlas_;
  TextureHandle back_atlas_;
  TextureHandle side_atlas_;
  TextureHandle front_side_atlas_;
  TextureHandle back_side_atlas_;
  TileSetHandle idle_frames_[8UL];
  TileSetHandle walk_frames_[8UL];
  TileSetHandle run_frames_[8UL];
  std::optional<Components> components_;
};

bool PlayerCharacter::AssignMovementTileSets(
  game::Assets& assets,
  TileSetHandle* movement_tilsets,
  std::size_t cardinal_start_offset,
  std::size_t ordinal_start_offset) const
{
  const std::array kFrameAssignments = {
    std::make_tuple(kFront, front_atlas_, TileOrientation::kNormal),
    std::make_tuple(kBack, back_atlas_, TileOrientation::kNormal),
    std::make_tuple(kRight, side_atlas_, TileOrientation::kNormal),
    std::make_tuple(kLeft, side_atlas_, TileOrientation::kFlipped),
    std::make_tuple(kFrontRight, front_side_atlas_, TileOrientation::kNormal),
    std::make_tuple(kFrontLeft, front_side_atlas_, TileOrientation::kFlipped),
    std::make_tuple(kBackRight, back_side_atlas_, TileOrientation::kNormal),
    std::make_tuple(kBackLeft, back_side_atlas_, TileOrientation::kFlipped),
  };

  for (const auto& [side_index, atlas, tile_orientation_x] : kFrameAssignments)
  {
    if (auto ok_or_error = assets.assign(
          movement_tilsets[side_index],
          atlas,
          TileSetSliceUniform{
            .tile_size_px = {64, 64},
            .tile_orientation_x = tile_orientation_x,
            .tile_orientation_y = TileOrientation::kNormal,
            .direction = TileSliceDirection::kRowWise,
            .start_offset = cardinal_start_offset,
            .stop_after = 6,
          });
        !ok_or_error)
    {
      SDE_LOG_ERROR("failed");
      return false;
    }
  }
  return true;
}

std::unique_ptr<ScriptRuntime> createPlayerCharacter() { return std::make_unique<PlayerCharacter>(); }