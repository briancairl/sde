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
    ImGui::Text("tile set: %lu", tile_map_active_options_.tile_set.id());

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
      auto id = assets.registry.create();
      assets.registry.emplace<TileMap>(id, tile_map_active_options_);
      assets.registry.emplace<Position>(id, Position{.center = {0, 0}});
      tile_map_active_ = id;
      tile_inspect_coords_.reset();
      tile_inspect_index_.reset();
    }

    if (tile_map_active_.has_value())
    {
      const auto& tf = assets.registry.get<TransformQuery>(transform_query_id_);
      const auto& pick_pos = sde::transform(tf.world_from_viewport, app.getMousePositionViewport());
      auto [tm, tm_pos] = assets.registry.get<TileMap, Position>(*tile_map_active_);
      const auto ti = tm.getTileIndex(pick_pos - tm_pos.center);

      ImGui::InputFloat2("origin", tm_pos.center.data());

      if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      {
        if (tm.within(ti))
        {
          tile_inspect_coords_ = ti;
        }
        else
        {
          tile_inspect_coords_.reset();
        }
        tile_inspect_index_.reset();
      }

      if (tm.within(ti) and ImGui::IsMouseClicked(ImGuiMouseButton_Left) and tile_inspect_index_.has_value())
      {
        tm[ti] = (*tile_inspect_index_);
      }
    }
    else
    {
      tile_inspect_coords_.reset();
    }

    if (
      tile_inspect_coords_.has_value() and ImGui::IsMouseClicked(ImGuiMouseButton_Right) and
      !ImGui::IsPopupOpen("tile-selection"))
    {
      ImGui::OpenPopup("tile-selection");
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
          }
        }
        ImGui::EndChild();
      }
      ImGui::EndPopup();
    }

    ImGui::End();

    return {};
  }
};

std::unique_ptr<ScriptRuntime> _TileMapEditor() { return std::make_unique<TileMapEditor>(); }