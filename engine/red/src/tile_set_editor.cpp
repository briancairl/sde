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
  float atlas_tile_display_width_ = 0;
  bool flip_horizontal_ = false;
  bool flip_vertical_ = false;
  std::vector<std::pair<int, Rect2f>> candidate_index_and_tiles_;
  std::vector<Rect2f> candidate_tiles_;
  int next_index_ = 0;

  bool onLoad(IArchive& ar) override
  {
    using namespace sde::serial;
    ar >> Field{"selected_tile_set", selected_tile_set_};
    ar >> Field{"atlas_texture_selected", atlas_texture_selected_};
    ar >> Field{"atlas_tile_size", atlas_tile_size_};
    ar >> Field{"atlas_tile_selected", atlas_tile_selected_};
    ar >> Field{"atlas_tile_display_width", atlas_tile_display_width_};
    return true;
  }

  bool onSave(OArchive& ar) const override
  {
    using namespace sde::serial;
    ar << Field{"selected_tile_set", selected_tile_set_};
    ar << Field{"atlas_texture_selected", atlas_texture_selected_};
    ar << Field{"atlas_tile_size", atlas_tile_size_};
    ar << Field{"atlas_tile_selected", atlas_tile_selected_};
    ar << Field{"atlas_tile_display_width", atlas_tile_display_width_};
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

  void onPreview(SharedAssets& assets, const Texture& texture)
  {
    candidate_index_and_tiles_.clear();
    const Vec2f tex_coord_rates = atlas_tile_size_.array().cast<float>() / texture.shape.value.array().cast<float>();

    for (int i = 0; i < atlas_tile_selected_.rows(); ++i)
    {
      for (int j = 0; j < atlas_tile_selected_.cols(); ++j)
      {
        if (const int index = atlas_tile_selected_(i, j); index > 0)
        {
          const Vec2f min_tex{tex_coord_rates.x() * (i + 0), tex_coord_rates.y() * (j + 0)};
          const Vec2f max_tex{tex_coord_rates.x() * (i + 1), tex_coord_rates.y() * (j + 1)};
          candidate_index_and_tiles_.emplace_back(index, Rect2f{min_tex, max_tex});
        }
      }
    }

    if (flip_vertical_)
    {
      for (auto& [index, rect] : candidate_index_and_tiles_)
      {
        std::swap(rect.pt0.y(), rect.pt1.y());
      }
    }

    if (flip_horizontal_)
    {
      for (auto& [index, rect] : candidate_index_and_tiles_)
      {
        std::swap(rect.pt0.x(), rect.pt1.x());
      }
    }

    if (!candidate_index_and_tiles_.empty())
    {
      candidate_tiles_.clear();
      std::sort(
        std::begin(candidate_index_and_tiles_),
        std::end(candidate_index_and_tiles_),
        [](const auto& lhs, const auto& rhs) { return std::get<0>(lhs) < std::get<0>(rhs); });
      candidate_tiles_.reserve(candidate_index_and_tiles_.size());
      std::transform(
        std::begin(candidate_index_and_tiles_),
        std::end(candidate_index_and_tiles_),
        std::back_inserter(candidate_tiles_),
        [](const auto& v) { return std::get<1>(v); });
    }
    Preview(candidate_tiles_, texture, toImVec2(atlas_tile_size_.cast<float>()), ImVec2{5.f, 5.f});
  }

  void onCreatePressed(SharedAssets& assets, const Texture& texture)
  {
    if (candidate_tiles_.empty())
    {
      return;
    }
    next_index_ = 0;
    if (auto ok_or_error = assets.graphics.tile_sets.create(atlas_texture_selected_, std::move(candidate_tiles_));
        !ok_or_error.has_value())
    {
      SDE_LOG_ERROR("failed to create tile set");
    }
    atlas_tile_selected_.setZero();
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

      if (ImGui::InputInt2("tile size (px)", atlas_tile_size_.data()) and (atlas_tile_size_.array() > 0).all())
      {
        const Vec2i dims = texture->shape.value.array() / atlas_tile_size_.array();
        atlas_tile_selected_.resize(dims.x(), dims.y());
        atlas_tile_selected_.setZero();
      }

      const auto max_display_width = std::max(1.F, ImGui::GetWindowWidth() - 2.F * ImGui::GetStyle().ScrollbarSize);
      if (atlas_tile_display_width_ < 1)
      {
        atlas_tile_display_width_ = max_display_width;
      }

      ImGui::SliderFloat("display width (px)", &atlas_tile_display_width_, max_display_width, 10000.0);
      ImGui::Checkbox("flip horizontal", &flip_horizontal_);
      ImGui::Checkbox("flip vertical", &flip_vertical_);

      onPreview(assets, *texture);

      if (ImGui::Button("create"))
      {
        onCreatePressed(assets, *texture);
      }
      ImGui::SameLine();
      if (ImGui::Button("reset"))
      {
        atlas_tile_selected_.setZero();
      }

      ImGui::BeginChild("#editor", ImVec2{max_display_width, 0.F}, false, ImGuiWindowFlags_HorizontalScrollbar);

      const ImVec2 atlas_texture_display_size{
        atlas_tile_display_width_, atlas_tile_display_width_ / texture->shape.aspect()};

      const auto atlas_texture_image_pos = ImGui::GetCursorScreenPos();
      ImGui::Image(
        reinterpret_cast<void*>(texture->native_id.value()), atlas_texture_display_size, ImVec2{0, 0}, ImVec2{1, 1});

      TileSetEditor::handleTextureDragAndDrop(assets);

      const float scaling = atlas_tile_display_width_ / texture->shape.value.x();
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
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
              if (atlas_tile_selected_(i, j) != 0)
              {
                --next_index_;
                atlas_tile_selected_(i, j) = 0;
              }
            }
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
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

      ImGui::EndChild();
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
      ImGui::BeginChild("tile-set", ImVec2{0.F, 80.F}, false, ImGuiWindowFlags_HorizontalScrollbar);
      Preview(*element, *atlas_texture, ImVec2{50.f, 50.f});
      if (ImGui::IsItemHovered())
      {
        const auto p_min = ImGui::GetItemRectMin();
        const auto p_max = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, ImColor{1.0F, 1.0F, 0.0F, 0.25F});
      }

      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        const ImVec4 tint =
          ImGui::SetDragDropPayload("SDE_TILESET_ASSET", std::addressof(handle), sizeof(handle), /*cond = */ 0)
          ? ImVec4{0, 1, 0, 1}
          : ImVec4{1, 1, 1, 1};
        ImGui::TextColored(tint, "tile-set[%lu]", handle.id());
        Preview(*element, *atlas_texture, ImVec2{25.f, 25.f}, ImVec2{5.f, 5.f}, 4UL);
        ImGui::EndDragDropSource();
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
