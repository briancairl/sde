#define SDE_SCRIPT_NAME "tile_set_editor"

// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"

// RED
#include "red/imgui_common.hpp"


using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

struct tile_set_editor
{
  TileSetHandle selected_tile_set;
  TextureHandle atlas_texture_selected;
  Vec2i atlas_tile_size = {32, 32};
  MatXi atlas_tile_selected = {};
  float atlas_tile_display_width = 0;
  bool flip_horizontal = false;
  bool flip_vertical = false;
  sde::vector<std::pair<int, Rect2f>> candidate_index_and_tiles;
  sde::vector<Rect2f> candidate_tiles;
  int next_index = 0;
};

void handle_drag_and_drop_texture(tile_set_editor* self, GameResources& resources)
{
  if (ImGui::BeginDragDropTarget())
  {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_TEXTURE_ASSET"))
    {
      SDE_ASSERT_EQ(payload->DataSize, sizeof(TextureHandle));
      if (const auto h = *reinterpret_cast<const TextureHandle*>(payload->Data); resources.exists(h))
      {
        self->atlas_texture_selected = h;
        self->atlas_tile_selected.resize(0, 0);
      }
      SDE_LOG_INFO_FMT("set atlas: texture[%lu]", self->atlas_texture_selected.id());
    }
    ImGui::EndDragDropTarget();
  }
}

void on_preview(tile_set_editor* self, GameResources& resources, const Texture& texture)
{
  self->candidate_index_and_tiles.clear();
  const Vec2f tex_coord_rates = self->atlas_tile_size.array().cast<float>() / texture.shape.value.array().cast<float>();

  for (int i = 0; i < self->atlas_tile_selected.rows(); ++i)
  {
    for (int j = 0; j < self->atlas_tile_selected.cols(); ++j)
    {
      if (const int index = self->atlas_tile_selected(i, j); index > 0)
      {
        const Vec2f min_tex{tex_coord_rates.x() * (i + 0), tex_coord_rates.y() * (j + 0)};
        const Vec2f max_tex{tex_coord_rates.x() * (i + 1), tex_coord_rates.y() * (j + 1)};
        self->candidate_index_and_tiles.emplace_back(index, Rect2f{min_tex, max_tex});
      }
    }
  }

  if (self->flip_vertical)
  {
    for (auto& [index, rect] : self->candidate_index_and_tiles)
    {
      std::swap(rect.pt0.y(), rect.pt1.y());
    }
  }

  if (self->flip_horizontal)
  {
    for (auto& [index, rect] : self->candidate_index_and_tiles)
    {
      std::swap(rect.pt0.x(), rect.pt1.x());
    }
  }

  if (!self->candidate_index_and_tiles.empty())
  {
    self->candidate_tiles.clear();
    std::sort(
      std::begin(self->candidate_index_and_tiles),
      std::end(self->candidate_index_and_tiles),
      [](const auto& lhs, const auto& rhs) { return std::get<0>(lhs) < std::get<0>(rhs); });
    self->candidate_tiles.reserve(self->candidate_index_and_tiles.size());
    std::transform(
      std::begin(self->candidate_index_and_tiles),
      std::end(self->candidate_index_and_tiles),
      std::back_inserter(self->candidate_tiles),
      [](const auto& v) { return std::get<1>(v); });
  }
  Preview(self->candidate_tiles, texture, toImVec2(self->atlas_tile_size.cast<float>()), ImVec2{5.f, 5.f});
}

void on_create_pressed(tile_set_editor* self, GameResources& resources, const Texture& texture)
{
  if (self->candidate_tiles.empty())
  {
    return;
  }
  self->next_index = 0;
  if (auto ok_or_error = resources.create<TileSetCache>(self->atlas_texture_selected, std::move(self->candidate_tiles));
      !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << "failed to create tile set: " << ok_or_error.error();
  }
  self->atlas_tile_selected.setZero();
}

void update_selector(tile_set_editor* self, GameResources& resources, const AppProperties& app)
{
  ImGui::Begin("tile-set-selector");
  if (const auto texture = resources.get_if(self->atlas_texture_selected))
  {
    if (self->atlas_tile_selected.size() == 0)
    {
      self->atlas_tile_size = (texture->shape.value.array() / 10);
      const Vec2i dims = texture->shape.value.array() / self->atlas_tile_size.array();
      self->atlas_tile_selected.resize(dims.x(), dims.y());
      self->atlas_tile_selected.setZero();
    }

    if (ImGui::InputInt2("tile size (px)", self->atlas_tile_size.data()) and (self->atlas_tile_size.array() > 0).all())
    {
      const Vec2i dims = texture->shape.value.array() / self->atlas_tile_size.array();
      self->atlas_tile_selected.resize(dims.x(), dims.y());
      self->atlas_tile_selected.setZero();
    }

    const auto max_display_width = std::max(1.F, ImGui::GetWindowWidth() - 2.F * ImGui::GetStyle().ScrollbarSize);
    if (self->atlas_tile_display_width < 1)
    {
      self->atlas_tile_display_width = max_display_width;
    }

    ImGui::SliderFloat("display width (px)", &self->atlas_tile_display_width, max_display_width, 10000.0);
    ImGui::Checkbox("flip horizontal", &self->flip_horizontal);
    ImGui::Checkbox("flip vertical", &self->flip_vertical);

    on_preview(self, resources, *texture);

    if (ImGui::Button("create"))
    {
      on_create_pressed(self, resources, *texture);
    }
    ImGui::SameLine();
    if (ImGui::Button("reset"))
    {
      self->atlas_tile_selected.setZero();
    }

    ImGui::BeginChild("#editor", ImVec2{max_display_width, 0.F}, false, ImGuiWindowFlags_HorizontalScrollbar);

    const ImVec2 atlas_texture_display_size{
      self->atlas_tile_display_width, self->atlas_tile_display_width / texture->shape.aspect()};

    const auto atlas_texture_image_pos = ImGui::GetCursorScreenPos();
    ImGui::Image(
      reinterpret_cast<void*>(texture->native_id.value()), atlas_texture_display_size, ImVec2{0, 0}, ImVec2{1, 1});

    handle_drag_and_drop_texture(self, resources);

    const float scaling = self->atlas_tile_display_width / texture->shape.value.x();
    const ImVec2 atlas_tile_display_size{scaling * self->atlas_tile_size.x(), scaling * self->atlas_tile_size.y()};

    auto* drawlist = ImGui::GetWindowDrawList();
    const ImColor tile_grid_border_color{ImGui::GetStyle().Colors[ImGuiCol_Border]};
    for (int i = 0; i < self->atlas_tile_selected.rows(); ++i)
    {
      for (int j = 0; j < self->atlas_tile_selected.cols(); ++j)
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
            if (self->atlas_tile_selected(i, j) != 0)
            {
              --self->next_index;
              self->atlas_tile_selected(i, j) = 0;
            }
          }
          else if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
          {
            drawlist->AddRectFilled(min_pos, max_pos, ImColor{1.0F, 0.0F, 0.0F, 0.3F});
            if (self->atlas_tile_selected(i, j) == 0)
            {
              ++self->next_index;
              self->atlas_tile_selected(i, j) = self->next_index;
            }
          }
          else
          {
            drawlist->AddRectFilled(min_pos, max_pos, ImColor{1.0F, 1.0F, 0.0F, 0.3F});
          }
        }
        else if (const int index = self->atlas_tile_selected(i, j); index > 0)
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
    self->atlas_texture_selected.reset();
    ImGui::Dummy(ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin());
    handle_drag_and_drop_texture(self, resources);
  }
  ImGui::End();
}

void update_previewer(tile_set_editor* self, GameResources& resources, const AppProperties& app)
{
  std::optional<TileSetHandle> delete_this_tile_set;

  ImGui::Begin("tile-set-previewer");
  for (const auto& [handle, element] : resources.get<TileSetCache>())
  {
    auto atlas_texture = resources(element->tile_atlas);
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
      self->selected_tile_set = handle;
    }
    static constexpr auto kPopUpFlags = ImGuiWindowFlags_None;
    if (ImGui::BeginPopup("tile-set-pop-up", kPopUpFlags))
    {
      ImGui::Text("tile-set[%lu]", self->selected_tile_set.id());
      ImGui::Separator();
      if (ImGui::Button("delete"))
      {
        delete_this_tile_set = self->selected_tile_set;
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
    resources.remove(*delete_this_tile_set);
  }
}

bool load(tile_set_editor* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  ar >> Field{"selected_tile_set", self->selected_tile_set};
  ar >> Field{"atlas_texture_selected", self->atlas_texture_selected};
  ar >> Field{"atlas_tile_size", self->atlas_tile_size};
  ar >> Field{"atlas_tile_selected", self->atlas_tile_selected};
  ar >> Field{"atlas_tile_display_width", self->atlas_tile_display_width};
  return true;
}


bool save(tile_set_editor* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  ar << Field{"selected_tile_set", self->selected_tile_set};
  ar << Field{"atlas_texture_selected", self->atlas_texture_selected};
  ar << Field{"atlas_tile_size", self->atlas_tile_size};
  ar << Field{"atlas_tile_selected", self->atlas_tile_selected};
  ar << Field{"atlas_tile_display_width", self->atlas_tile_display_width};
  return true;
}


bool initialize(tile_set_editor* self, GameResources& resources, const AppProperties& app) { return true; }

bool update(tile_set_editor* self, GameResources& resources, const AppProperties& app)
{
  update_selector(self, resources, app);

  if (ImGui::BeginDragDropTarget())
  {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_TEXTURE_ASSET"))
    {
      SDE_ASSERT_EQ(payload->DataSize, sizeof(TextureHandle));
      if (const auto h = *reinterpret_cast<const TextureHandle*>(payload->Data); resources.exists(h))
      {
        self->atlas_texture_selected = h;
        self->atlas_tile_selected.resize(0, 0);
      }
      SDE_LOG_INFO() << "set atlas: texture[" << self->atlas_texture_selected << ']';
    }
    ImGui::EndDragDropTarget();
  }

  update_previewer(self, resources, app);

  return true;
}

SDE_NATIVE_SCRIPT__REGISTER_AUTO(tile_set_editor);
