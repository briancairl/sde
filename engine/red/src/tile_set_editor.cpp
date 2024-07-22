// C++ Standard Library
#include <ostream>

// ImGui
#include <imgui.h>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// RED
#include "red/tile_set_editor.hpp"

using namespace sde;
using namespace sde::game;


class TileSetEditor final : public ScriptRuntime
{
public:
  TileSetEditor() : ScriptRuntime{"TileSetEditor"} {}

private:
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

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override { return true; }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets->contains<ImGuiContext*>())
    {
      return make_unexpected(ScriptError::kNonCriticalUpdateFailure);
    }

    ImGui::Begin("tile_sets");
    for (const auto& [handle, element] : assets.graphics.tile_sets)
    {
      ImGui::Text("TileSet[%lu] from (%lu)", handle.id(), element->tile_atlas.id());
      auto atlas_texture = assets.graphics.textures(element->tile_atlas);
      for (const auto& bounds : element->tile_bounds)
      {
        static constexpr float kTileWidth = 100.0F;
        ImGui::Image(
          reinterpret_cast<void*>(atlas_texture->native_id.value()),
          ImVec2{kTileWidth, kTileWidth},
          ImVec2{bounds.max().x(), bounds.max().y()},
          ImVec2{bounds.min().x(), bounds.min().y()});
        ImGui::SameLine();
      }
      ImGui::NewLine();
    }
    ImGui::End();
    return {};
  }
};

std::unique_ptr<ScriptRuntime> _TileSetEditor() { return std::make_unique<TileSetEditor>(); }
