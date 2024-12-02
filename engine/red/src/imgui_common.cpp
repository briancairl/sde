// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/graphics/texture.hpp"
#include "sde/graphics/tile_set.hpp"

// RED
#include "red/imgui_common.hpp"


bool Preview(const sde::graphics::Texture& texture, const ImVec2& preview_size)
{
  const ImColor border_color{ImGui::GetStyle().Colors[ImGuiCol_Border]};
  auto* drawlist = ImGui::GetWindowDrawList();
  const auto pos = ImGui::GetCursorScreenPos();
  ImGui::Dummy(preview_size);

  drawlist->AddRect(pos, pos + preview_size, border_color);
  if (texture.shape.width() > texture.shape.height())
  {
    const float aspect = static_cast<float>(texture.shape.height()) / static_cast<float>(texture.shape.width());
    const ImVec2 display_size{preview_size.x, preview_size.x * aspect};
    const ImVec2 centering{(preview_size - display_size) * 0.5F};
    drawlist->AddImage(
      reinterpret_cast<void*>(texture.native_id.value()),
      pos + centering,
      pos + centering + display_size,
      ImVec2{0, 0},
      ImVec2{1, 1},
      IM_COL32_WHITE);
  }
  else
  {
    const float aspect = static_cast<float>(texture.shape.width()) / static_cast<float>(texture.shape.height());
    const ImVec2 display_size{preview_size.y * aspect, preview_size.y};
    const ImVec2 centering{(preview_size - display_size) * 0.5F};
    drawlist->AddImage(
      reinterpret_cast<void*>(texture.native_id.value()),
      pos + centering,
      pos + centering + display_size,
      ImVec2{0, 0},
      ImVec2{1, 1},
      IM_COL32_WHITE);
  }
  return false;
}


void Preview(
  const sde::vector<sde::Rect2f>& texcoords,
  const sde::graphics::Texture& texture,
  const ImVec2& preview_tile_size,
  const ImVec2& preview_tile_spacing,
  std::size_t max_tile_count)
{
  const float n_tiles = (max_tile_count == 0) ? texcoords.size() : std::min(max_tile_count, texcoords.size());
  const float alpha_decay = (max_tile_count == 0) ? 0.F : (1.F / n_tiles);

  const ImColor border_color{ImGui::GetStyle().Colors[ImGuiCol_Border]};
  auto* drawlist = ImGui::GetWindowDrawList();
  const auto origin = ImGui::GetCursorScreenPos();

  const ImVec2 full_size{(preview_tile_size.x + preview_tile_spacing.x) * n_tiles, preview_tile_size.y};
  ImGui::Dummy(full_size);

  for (std::size_t i = 0; i < n_tiles; ++i)
  {
    const auto& bounds = texcoords[i];
    const ImVec2 pos{origin.x + (preview_tile_size.x + preview_tile_spacing.x) * i, origin.y};
    drawlist->AddRect(pos, pos + preview_tile_size, border_color);

    const sde::Vec2f extents{(bounds.pt1 - bounds.pt0).array().abs() * texture.shape.value.array().cast<float>()};
    if (extents.x() > extents.y())
    {
      const float aspect = extents.y() / extents.x();
      const ImVec2 display_size{preview_tile_size.x, preview_tile_size.x * aspect};
      const ImVec2 centering{(preview_tile_size - display_size) * 0.5F};
      drawlist->AddImage(
        reinterpret_cast<void*>(texture.native_id.value()),
        pos + centering,
        pos + centering + display_size,
        toImVec2(bounds.pt0),
        toImVec2(bounds.pt1),
        ImColor{1.f, 1.f, 1.f, (1.f - alpha_decay * i)});
    }
    else
    {
      const float aspect = extents.x() / extents.y();
      const ImVec2 display_size{preview_tile_size.y * aspect, preview_tile_size.y};
      const ImVec2 centering{(preview_tile_size - display_size) * 0.5F};
      drawlist->AddImage(
        reinterpret_cast<void*>(texture.native_id.value()),
        pos + centering,
        pos + centering + display_size,
        toImVec2(bounds.pt0),
        toImVec2(bounds.pt1),
        ImColor{1.f, 1.f, 1.f, (1.f - alpha_decay * i)});
    }
  }
}


void Preview(
  const sde::graphics::TileSet& tileset,
  const sde::graphics::Texture& texture,
  const ImVec2& preview_tile_size,
  const ImVec2& preview_tile_spacing,
  std::size_t max_tile_count)
{
  Preview(tileset.tile_bounds, texture, preview_tile_size, preview_tile_spacing, max_tile_count);
}
