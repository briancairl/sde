// C++ Standard Library
#include <vector>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/asset.hpp"
#include "sde/format.hpp"
#include "sde/graphics/texture_fwd.hpp"
#include "sde/graphics/tile_set_fwd.hpp"
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
    else if constexpr (std::is_same_v<U, sde::Rect2f>)
    {
      ImGui::Text(
        "%s : [(%.02f x %.02f), (%.02f x %.02f)]",
        field.name,
        field->pt0.x(),
        field->pt0.y(),
        field->pt1.x(),
        field->pt1.y());
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


bool Preview(const sde::graphics::Texture& texture, const ImVec2& preview_size);


void Preview(
  const sde::vector<sde::Rect2f>& texcoords,
  const sde::graphics::Texture& texture,
  const ImVec2& preview_tile_size,
  const ImVec2& preview_tile_spacing = ImVec2{10, 10},
  std::size_t max_tile_count = 0);


void Preview(
  const sde::graphics::TileSet& tileset,
  const sde::graphics::Texture& texture,
  const ImVec2& preview_tile_size,
  const ImVec2& preview_tile_spacing = ImVec2{10, 10},
  std::size_t max_tile_count = 0);