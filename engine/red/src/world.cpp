// C++ Standard Library
#include <optional>
#include <ostream>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/logging.hpp"

// RED
#include "red/components/common.hpp"
#include "red/world.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;


class Components : public Resource<Components>
{
  friend fundemental_type;

public:
  explicit Components(entt::entity id, entt::registry& reg) : id_{id}, registry_{&reg}
  {
    registry_->emplace<Background>(id_);
    registry_->emplace<Position>(id_, Position{.center = Vec2f::Zero()});
    registry_->emplace<TileMap>(id_, TileMap{TileMapOptions{}});
  }

private:
  entt::entity id_;
  entt::registry* registry_;

  auto field_list()
  {
    return FieldList(Field{"position", registry_->get<Position>(id_)}, Field{"tile_map", registry_->get<TileMap>(id_)});
  }
};


class World final : public ScriptRuntime
{
public:
  World() : ScriptRuntime{"World"} {}

private:
  EntityHandle entity_;
  std::optional<Components> components_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    // ar >> Field{"world_map", world_map_};
    return true;
  }

  bool onSave(OArchive& ar, const SharedAssets& assets) const override
  {
    using namespace sde::serial;
    // ar << Field{"world_map", world_map_};
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
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
    return {};
  }
};

std::unique_ptr<ScriptRuntime> createWorld() { return std::make_unique<World>(); }
