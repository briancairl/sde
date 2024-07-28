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
    if (depth > 0)
    {
      ImGui::Dummy(ImVec2(depth * 10, 0.0));
      ImGui::SameLine();
    }

    using U = std::remove_const_t<T>;
    if constexpr (std::is_same_v<U, sde::asset::path>)
    {
      ImGui::Text("%s : %s", field.name, field->string().c_str());
    }
    else if constexpr (std::is_same_v<U, sde::Vec2i>)
    {
      ImGui::Text("%s : (%d x %d)", field.name, field->x(), field->y());
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
    else
    {
      ImGui::Text("%s : ...", field.name);
    }
    return true;
  }
};


inline bool TexturePreviewImage(
  const sde::graphics::Texture& texture,
  const ImVec2& preview_size,
  const ImColor& hovering_tint_color = ImColor{1.0F, 1.0F, 0.0F, 0.2F})
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
  if (ImGui::IsMouseHoveringRect(pos, pos + preview_size))
  {
    drawlist->AddRectFilled(pos, pos + preview_size, hovering_tint_color);
    return true;
  }
  return false;
}
