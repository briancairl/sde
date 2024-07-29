// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/geometry.hpp"
#include "sde/logging.hpp"

// RED
#include "red/imgui_common.hpp"
#include "red/tile_set_editor.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;


class TileSetEditor final : public ScriptRuntime
{
public:
  TileSetEditor() : ScriptRuntime{"TileSetEditor"} {}

private:
  TileSetHandle selected_tile_set_;
  TextureHandle atlas_texture_selected_;
  Vec2i atlas_tile_size_ = {32, 32};
  MatXi atlas_tile_selected_ = {};
  int next_index_ = 0;

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

  void handleTextureDragAndDrop(SharedAssets& assets)
  {
    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_TEXTURE_ASSET"))
      {
        SDE_ASSERT_EQ(payload->DataSize, sizeof(TextureHandle));
        if (const auto h = *reinterpret_cast<const TextureHandle*>(payload->Data); assets.graphics.textures.exists(h))
        {
          atlas_texture_selected_ = h;
          atlas_tile_selected_.resize(0, 0);
        }
        SDE_LOG_INFO_FMT("set atlas: texture[%lu]", atlas_texture_selected_.id());
      }
      ImGui::EndDragDropTarget();
    }
  }

  void onCreatePressed(SharedAssets& assets, const Texture& texture)
  {
    next_index_ = 0;
    const Vec2f tex_coord_rates = atlas_tile_size_.array().cast<float>() / texture.shape.value.array().cast<float>();
    std::vector<std::pair<int, Bounds2f>> indexed_bounds;
    for (int i = 0; i < atlas_tile_selected_.rows(); ++i)
    {
      for (int j = 0; j < atlas_tile_selected_.cols(); ++j)
      {
        if (const int index = atlas_tile_selected_(i, j); index > 0)
        {
          const Vec2f min_tex{tex_coord_rates.x() * (i + 0), tex_coord_rates.y() * (j + 0)};
          const Vec2f max_tex{tex_coord_rates.x() * (i + 1), tex_coord_rates.y() * (j + 1)};
          indexed_bounds.emplace_back(index, Bounds2f{min_tex, max_tex});
          atlas_tile_selected_(i, j) = 0;
        }
      }
    }
    if (!indexed_bounds.empty())
    {
      std::sort(std::begin(indexed_bounds), std::end(indexed_bounds), [](const auto& lhs, const auto& rhs) {
        return std::get<0>(lhs) < std::get<0>(rhs);
      });
      std::vector<Bounds2f> bounds;
      bounds.reserve(indexed_bounds.size());
      std::transform(
        std::begin(indexed_bounds), std::end(indexed_bounds), std::back_inserter(bounds), [](const auto& v) {
          return std::get<1>(v);
        });
      if (auto ok_or_error = assets.graphics.tile_sets.create(atlas_texture_selected_, std::move(bounds));
          !ok_or_error.has_value())
      {
        SDE_LOG_ERROR("failed to create tile set");
      }
    }
  }

  void updateTileSelector(SharedAssets& assets, const AppProperties& app)
  {
    ImGui::Begin("tile-set-selector");
    if (const auto texture = assets.graphics.textures.get_if(atlas_texture_selected_))
    {
      if (atlas_tile_selected_.size() == 0)
      {
        atlas_tile_size_ = (texture->shape.value.array() / 10);
        const Vec2i dims = texture->shape.value.array() / atlas_tile_size_.array();
        atlas_tile_selected_.resize(dims.x(), dims.y());
        atlas_tile_selected_.setZero();
      }

      if (ImGui::Button("create"))
      {
        onCreatePressed(assets, *texture);
      }

      ImGui::SameLine();

      if (ImGui::InputInt2(
            "tile size (px)",
            atlas_tile_size_.data(),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AlwaysOverwrite))
      {
        const Vec2i dims = texture->shape.value.array() / atlas_tile_size_.array();
        atlas_tile_selected_.resize(dims.x(), dims.y());
        atlas_tile_selected_.setZero();
      }

      const auto atlas_display_width = std::max(1.F, ImGui::GetWindowWidth() - 2.F * ImGui::GetStyle().ScrollbarSize);
      const auto atlas_texture_image_pos = ImGui::GetCursorScreenPos();

      const ImVec2 atlas_texture_display_size{atlas_display_width, atlas_display_width * texture->shape.aspect()};
      ImGui::Image(
        reinterpret_cast<void*>(texture->native_id.value()), atlas_texture_display_size, ImVec2{0, 0}, ImVec2{1, 1});

      TileSetEditor::handleTextureDragAndDrop(assets);

      const float scaling = atlas_display_width / texture->shape.value.x();
      const ImVec2 atlas_tile_display_size{scaling * atlas_tile_size_.x(), scaling * atlas_tile_size_.y()};

      auto* drawlist = ImGui::GetWindowDrawList();
      const ImColor tile_grid_border_color{ImGui::GetStyle().Colors[ImGuiCol_Border]};
      for (int i = 0; i < atlas_tile_selected_.rows(); ++i)
      {
        for (int j = 0; j < atlas_tile_selected_.cols(); ++j)
        {
          const ImVec2 min_pos = atlas_texture_image_pos +
            ImVec2{
              static_cast<float>(atlas_tile_display_size.x * (i + 0)),
              static_cast<float>(atlas_tile_display_size.y * (j + 0))};
          const ImVec2 max_pos = atlas_texture_image_pos +
            ImVec2{
              static_cast<float>(atlas_tile_display_size.x * (i + 1)),
              static_cast<float>(atlas_tile_display_size.y * (j + 1))};
          if (ImGui::IsMouseHoveringRect(min_pos, max_pos))
          {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
              if (atlas_tile_selected_(i, j) != 0)
              {
                --next_index_;
                atlas_tile_selected_(i, j) = 0;
              }
            }
            else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            {
              drawlist->AddRectFilled(min_pos, max_pos, ImColor{1.0F, 0.0F, 0.0F, 0.3F});
              if (atlas_tile_selected_(i, j) == 0)
              {
                ++next_index_;
                atlas_tile_selected_(i, j) = next_index_;
              }
            }
            else
            {
              drawlist->AddRectFilled(min_pos, max_pos, ImColor{1.0F, 1.0F, 0.0F, 0.3F});
            }
          }
          else if (const int index = atlas_tile_selected_(i, j); index > 0)
          {
            drawlist->AddRectFilled(min_pos, max_pos, ImColor{0.0F, 1.0F, 1.0F, 0.3F});
            drawlist->AddText(min_pos, ImColor{1.0F, 0.0F, 0.0F, 1.0F}, format("%d", index));
          }
          drawlist->AddRect(min_pos, max_pos, tile_grid_border_color);
        }
      }
    }
    else
    {
      atlas_texture_selected_.reset();
      ImGui::Dummy(ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin());
      TileSetEditor::handleTextureDragAndDrop(assets);
    }
    ImGui::End();
  }

  void updateTileSetPreviewer(SharedAssets& assets, const AppProperties& app)
  {
    std::optional<TileSetHandle> delete_this_tile_set;

    ImGui::Begin("tile-set-previewer");
    for (const auto& [handle, element] : assets.graphics.tile_sets)
    {
      auto atlas_texture = assets.graphics.textures(element->tile_atlas);
      if (!atlas_texture)
      {
        ImGui::Text("tile-set[%lu] from texture[%lu] (MISSING!)", handle.id(), element->tile_atlas.id());
        continue;
      }

      ImGui::PushID(handle.id());
      ImGui::BeginChild("tile-set", ImVec2{0.F, 100.F}, true, ImGuiWindowFlags_HorizontalScrollbar);
      {
        const auto tile_display_height =
          std::max(1.F, ImGui::GetWindowHeight() - 2.F * ImGui::GetStyle().ScrollbarSize);
        for (const auto& bounds : element->tile_bounds)
        {
          ImGui::Image(
            reinterpret_cast<void*>(atlas_texture->native_id.value()),
            ImVec2{tile_display_height, tile_display_height},
            ImVec2{bounds.min().x(), bounds.min().y()},
            ImVec2{bounds.max().x(), bounds.max().y()});

          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
          {
            const ImVec4 tint =
              ImGui::SetDragDropPayload("SDE_TILESET_ASSET", std::addressof(handle), sizeof(handle), /*cond = */ 0)
              ? ImVec4{0, 1, 0, 1}
              : ImVec4{1, 1, 1, 1};
            ImGui::TextColored(tint, "tile-set[%lu]", handle.id());

            static constexpr std::size_t kPreviewTilesCount{4UL};
            static constexpr float kPreviewTilesSize{25.F};
            for (std::size_t i = 0; i < std::min(kPreviewTilesCount, element->tile_bounds.size()); ++i)
            {
              const auto& bounds = element->tile_bounds[i];
              const float alpha = static_cast<float>(kPreviewTilesCount - i) / static_cast<float>(kPreviewTilesCount);
              ImGui::Image(
                reinterpret_cast<void*>(atlas_texture->native_id.value()),
                ImVec2{kPreviewTilesSize, kPreviewTilesSize},
                ImVec2{bounds.min().x(), bounds.min().y()},
                ImVec2{bounds.max().x(), bounds.max().y()},
                ImVec4{1.0F, 1.0F, 1.0F, alpha});
              ImGui::SameLine();
            }
            ImGui::EndDragDropSource();
          }

          ImGui::SameLine();
        }
      }
      ImGui::EndChild();

      if (ImGui::IsItemClicked(ImGuiMouseButton_Right) and !ImGui::IsPopupOpen("tile-set-pop-up"))
      {
        ImGui::OpenPopup("tile-set-pop-up");
        selected_tile_set_ = handle;
      }
      static constexpr auto kPopUpFlags = ImGuiWindowFlags_None;
      if (ImGui::BeginPopup("tile-set-pop-up", kPopUpFlags))
      {
        ImGui::Text("tile-set[%lu]", selected_tile_set_.id());
        ImGui::Separator();
        if (ImGui::Button("delete"))
        {
          delete_this_tile_set = selected_tile_set_;
          ImGui::CloseCurrentPopup();
        }
        Visit(element, ImGuiFieldFormatter{});
        ImGui::EndPopup();
      }
      ImGui::PopID();
    }
    ImGui::End();

    if (delete_this_tile_set)
    {
      assets.graphics.tile_sets.remove(*delete_this_tile_set);
    }
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets->contains<ImGuiContext*>())
    {
      return make_unexpected(ScriptError::kNonCriticalUpdateFailure);
    }

    TileSetEditor::updateTileSelector(assets, app);

    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_TEXTURE_ASSET"))
      {
        SDE_ASSERT_EQ(payload->DataSize, sizeof(TextureHandle));
        if (const auto h = *reinterpret_cast<const TextureHandle*>(payload->Data); assets.graphics.textures.exists(h))
        {
          atlas_texture_selected_ = h;
          atlas_tile_selected_.resize(0, 0);
        }
        SDE_LOG_INFO_FMT("set atlas: texture[%lu]", atlas_texture_selected_.id());
      }
      ImGui::EndDragDropTarget();
    }

    TileSetEditor::updateTileSetPreviewer(assets, app);

    return {};
  }
};

std::unique_ptr<ScriptRuntime> _TileSetEditor() { return std::make_unique<TileSetEditor>(); }
