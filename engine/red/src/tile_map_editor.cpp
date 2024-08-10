// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/geometry.hpp"
#include "sde/geometry_utils.hpp"
#include "sde/graphics/tile_map.hpp"
#include "sde/logging.hpp"

// RED
#include "red/components.hpp"
#include "red/imgui_common.hpp"
#include "red/tile_map_editor.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;


class TileMapEditor final : public ScriptRuntime
{
public:
  TileMapEditor() : ScriptRuntime{"TileMapEditor"} {}

private:
  entt::entity transform_query_id_;
  std::optional<entt::entity> tile_map_active_;
  std::optional<Vec2i> tile_inspect_coords_;
  std::optional<std::size_t> tile_inspect_index_;
  TileMapOptions tile_map_active_options_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    return true;
  }

  bool onSave(OArchive& ar, const SharedAssets& assets) const override
  {
    using namespace sde::serial;
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    transform_query_id_ = assets.registry.create();
    assets.registry.emplace<TransformQuery>(transform_query_id_);
    return true;
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets->contains<ImGuiContext*>())
    {
      return make_unexpected(ScriptError::kNonCriticalUpdateFailure);
    }

    ImGui::Begin("tile-map-creator");

    ImGui::ColorEdit4("tint color", tile_map_active_options_.tint_color.data());
    ImGui::InputInt2("shape", tile_map_active_options_.shape.data());
    ImGui::InputFloat2("tile size", tile_map_active_options_.tile_size.data());

    if (auto h = tile_map_active_options_.tile_set; !h.isValid())
    {
      ImGui::TextUnformatted("tile-set: not set");
    }
    else if (auto tile_set = assets.graphics.tile_sets(h); !tile_set)
    {
      ImGui::TextUnformatted("tile-set: missing");
    }
    else if (auto tile_set_atlas_texture = assets.graphics.textures(tile_set->tile_atlas); !tile_set_atlas_texture)
    {
      ImGui::TextUnformatted("tile-set: missing atlas texture");
    }
    else
    {
      ImGui::Text("tile-set[%lu] (atlas texture[%lu])", h.id(), tile_set->tile_atlas.id());
      Preview(*tile_set, *tile_set_atlas_texture, ImVec2{50.f, 50.f}, ImVec2{5.f, 5.f}, 4);
    }

    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_TILESET_ASSET"))
      {
        SDE_ASSERT_EQ(payload->DataSize, sizeof(TileSetHandle));
        if (const auto h = *reinterpret_cast<const TileSetHandle*>(payload->Data); assets.graphics.tile_sets.exists(h))
        {
          tile_map_active_options_.tile_set = h;
        }
        SDE_LOG_INFO_FMT("set atlas: texture[%lu]", tile_map_active_options_.tile_set.id());
      }
      ImGui::EndDragDropTarget();
    }

    if (ImGui::Button("create") and tile_map_active_options_.tile_set.isValid())
    {
      auto value_or_error = assets.entities.create();
      if (value_or_error.has_value())
      {
        auto& e = *value_or_error;
        assets.entities.attach<TileMap>(e.handle, tile_map_active_options_);
        assets.entities.attach<Position>(e.handle, Position{.center = {0, 0}});
        assets.entities.attach<DebugWireFrame>(e.handle, DebugWireFrame{.color = Vec4f{1.F, 0.F, 0.F, 1.F}});
        tile_map_active_ = e->id;
        tile_inspect_coords_.reset();
        tile_inspect_index_.reset();
      }
    }

    const auto& tf = assets.registry.get<TransformQuery>(transform_query_id_);
    const auto& pick_pos = sde::transform(tf.world_from_viewport, app.getMousePositionViewport());

    if (tile_map_active_.has_value())
    {
      auto [tm, tm_pos] = assets.registry.get<TileMap, Position>(*tile_map_active_);
      const auto ti = tm.getTileIndex(pick_pos - tm_pos.center);

      ImGui::InputFloat2("origin", tm_pos.center.data());
      if (app.keys.isPressed(KeyCode::kA))
      {
        tm_pos.center.x() -= tile_map_active_options_.tile_size.x();
      }
      if (app.keys.isPressed(KeyCode::kD))
      {
        tm_pos.center.x() += tile_map_active_options_.tile_size.x();
      }
      if (app.keys.isPressed(KeyCode::kW))
      {
        tm_pos.center.y() += tile_map_active_options_.tile_size.y();
      }
      if (app.keys.isPressed(KeyCode::kS))
      {
        tm_pos.center.y() -= tile_map_active_options_.tile_size.y();
      }

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      {
        if (tm.within(ti))
        {
          tile_inspect_coords_ = ti;
          if (!ImGui::IsPopupOpen("tile-selection"))
          {
            ImGui::OpenPopup("tile-selection");
          }
        }
        else
        {
          tile_inspect_coords_.reset();
          if (!ImGui::IsPopupOpen("tile-map-edit"))
          {
            ImGui::OpenPopup("tile-map-edit");
          }
        }
        tile_inspect_index_.reset();
      }

      if (tm.within(ti) and ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) and tile_inspect_index_.has_value())
      {
        tm[ti] = (*tile_inspect_index_);
      }

      static constexpr auto kPopUpFlags = ImGuiWindowFlags_None;
      if (ImGui::BeginPopup("tile-selection", kPopUpFlags))
      {
        auto& tm = assets.registry.get<TileMap>(*tile_map_active_);

        if (auto tile_set = assets.graphics.tile_sets(tm.options().tile_set); !tile_set)
        {
          ImGui::Text("%s", "missing tilset");
        }
        else if (auto tile_set_atlas_texture = assets.graphics.textures(tile_set->tile_atlas); !tile_set_atlas_texture)
        {
          ImGui::Text("%s", "missing tilset altas");
        }
        else
        {
          static constexpr float kTileHeightPx = 50.F;
          const float aspect = tm.options().tile_size.y() / tm.options().tile_size.x();
          ImGui::BeginChild(
            "tile-browser",
            ImVec2{
              kTileHeightPx * aspect + 2.F * ImGui::GetStyle().ScrollbarSize,
              std::min(tile_set->tile_bounds.size(), 3UL) * kTileHeightPx});
          for (std::size_t tile_index = 0; tile_index < tile_set->tile_bounds.size(); ++tile_index)
          {
            const auto& bounds = tile_set->tile_bounds[tile_index];
            ImGui::Image(
              reinterpret_cast<void*>(tile_set_atlas_texture->native_id.value()),
              ImVec2{kTileHeightPx * aspect, kTileHeightPx},
              ImVec2{bounds.min().x(), bounds.min().y()},
              ImVec2{bounds.max().x(), bounds.max().y()});
            if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
            {
              tm[*tile_inspect_coords_] = tile_index;
              tile_inspect_index_ = tile_index;
              ImGui::CloseCurrentPopup();
            }
          }
          ImGui::EndChild();
        }
        ImGui::EndPopup();
      }

      if (ImGui::BeginPopup("tile-map-edit", kPopUpFlags))
      {

        ImGui::EndPopup();
      }
    }
    else
    {
      tile_inspect_coords_.reset();
    }

    assets.registry.view<TileMap, Position, DebugWireFrame>().each(
      [this, &pick_pos](entt::entity tm_id, const TileMap& tm, const Position& pos, DebugWireFrame& wireframe) {
        if (
          Bounds2f{Vec2f::Zero(), tm.mapSize()}.contains(pick_pos - pos.center) and
          ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
          tile_map_active_ = tm_id;
          tile_map_active_options_ = tm.options();
          wireframe.color = Vec4f{1.F, 0.F, 0.F, 1.F};
        }
        else if (tile_map_active_ != tm_id)
        {
          wireframe.color = Vec4f{0.F, 0.F, 0.F, 0.F};
        }
      });

    ImGui::End();

    return {};
  }
};

std::unique_ptr<ScriptRuntime> _TileMapEditor() { return std::make_unique<TileMapEditor>(); }
