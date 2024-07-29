// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/asset.hpp"
#include "sde/format.hpp"
#include "sde/graphics/texture.hpp"
#include "sde/resource.hpp"
#include "sde/resource_handle.hpp"


inline ImVec2 toImVec2(const sde::Vec2f& v) { return {v.x(), v.y()}; }

struct ImGuiFieldFormatter
{
  template <typename T> bool operator()(std::size_t depth, const sde::BasicField<T>& field)
  {
    using U = std::remove_const_t<T>;
    if (!sde::is_iterable_v<U> and depth > 0)
    {
      ImGui::Dummy(ImVec2(depth * 10, 0.0));
      ImGui::SameLine();
    }

    if constexpr (std::is_same_v<U, sde::asset::path>)
    {
      ImGui::Text("%s : %s", field.name, field->string().c_str());
    }
    else if constexpr (std::is_same_v<U, sde::Vec2i>)
    {
      ImGui::Text("%s : (%d x %d)", field.name, field->x(), field->y());
    }
    else if constexpr (std::is_same_v<U, sde::Bounds2f>)
    {
      ImGui::Text(
        "%s : [(%.02f x %.02f), (%.02f x %.02f)]",
        field.name,
        field->min().x(),
        field->min().y(),
        field->max().x(),
        field->max().y());
    }
    else if constexpr (std::is_same_v<U, sde::Hash>)
    {
      ImGui::Text("%s : {%lu}", field.name, field->value);
    }
    else if constexpr (std::is_enum_v<U>)
    {
      ImGui::Text("%s : %d", field.name, static_cast<int>(field.get()));
    }
    else if constexpr (std::is_integral_v<U>)
    {
      ImGui::Text("%s : %d", field.name, static_cast<int>(field.get()));
    }
    else if constexpr (sde::is_resource_handle_v<T>)
    {
      ImGui::Text("%s : %d", field.name, static_cast<int>(field.get().id()));
    }
    else if constexpr (sde::is_resource_v<T>)
    {
      return ImGui::CollapsingHeader(sde::format("%s : ...", field.name));
    }
    else if constexpr (sde::is_iterable_v<U>)
    {
      std::size_t index = 0;
      for (const auto& v : field.get())
      {
        sde::Visit(sde::Field{sde::format("%s[%lu]", field.name, index), v}, ImGuiFieldFormatter{}, depth);
        ++index;
      }
    }
    else
    {
      ImGui::Text("%s : ...", field.name);
    }
    return true;
  }
};


inline bool Preview(const sde::graphics::Texture& texture, const ImVec2& preview_size)
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


inline void Preview(
  const sde::graphics::TileSet& tileset,
  const sde::graphics::Texture& texture,
  const ImVec2& preview_tile_size,
  const ImVec2& preview_tile_spacing = ImVec2{10, 10},
  std::size_t max_tile_count = 0)
{
  const float n_tiles =
    (max_tile_count == 0) ? tileset.tile_bounds.size() : std::min(max_tile_count, tileset.tile_bounds.size());
  const float alpha_decay = (max_tile_count == 0) ? 0.F : (1.F / n_tiles);

  const ImColor border_color{ImGui::GetStyle().Colors[ImGuiCol_Border]};
  auto* drawlist = ImGui::GetWindowDrawList();
  const auto origin = ImGui::GetCursorScreenPos();

  const ImVec2 full_size{(preview_tile_size.x + preview_tile_spacing.x) * n_tiles, preview_tile_size.y};
  ImGui::Dummy(full_size);

  for (std::size_t i = 0; i < n_tiles; ++i)
  {
    const auto& bounds = tileset.tile_bounds[i];
    const ImVec2 pos{origin.x + (preview_tile_size.x + preview_tile_spacing.x) * i, origin.y};
    drawlist->AddRect(pos, pos + preview_tile_size, border_color);

    const sde::Vec2f extents{bounds.max() - bounds.min()};
    if (extents.x() > extents.y())
    {
      const float aspect = extents.y() / extents.x();
      const ImVec2 display_size{preview_tile_size.x, preview_tile_size.x * aspect};
      const ImVec2 centering{(preview_tile_size - display_size) * 0.5F};
      drawlist->AddImage(
        reinterpret_cast<void*>(texture.native_id.value()),
        pos + centering,
        pos + centering + display_size,
        toImVec2(bounds.min()),
        toImVec2(bounds.max()),
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
        toImVec2(bounds.min()),
        toImVec2(bounds.max()),
        ImColor{1.f, 1.f, 1.f, (1.f - alpha_decay * i)});
    }
  }
}
