#define SDE_SCRIPT_TYPE_NAME "texture_viewer"

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


struct texture_viewer : native_script_data
{
  TextureHandle selected_texture;
};

template <typename ArchiveT> bool serialize(texture_viewer* self, ArchiveT& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(texture_viewer* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool shutdown(texture_viewer* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

bool update(texture_viewer* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  std::optional<TextureHandle> delete_this_texture;

  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }

  ImGui::Begin("textures");
  for (const auto& [handle, element] : resources.get<graphics::TextureCache>())
  {
    if (element->source_image.isNull())
    {
      continue;
    }
    ImGui::PushID(handle.id());

    const auto max_size_x = std::max(1.F, ImGui::GetWindowWidth() - 2.F * ImGui::GetStyle().ScrollbarSize);
    Preview(*element, {max_size_x, max_size_x});
    if (ImGui::IsItemHovered())
    {
      const auto p_min = ImGui::GetItemRectMin();
      const auto p_max = ImGui::GetItemRectMax();
      ImGui::GetWindowDrawList()->AddRectFilled(p_min, p_max, ImColor{1.0F, 1.0F, 0.0F, 0.25F});
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
      const ImVec4 tint =
        ImGui::SetDragDropPayload("SDE_TEXTURE_ASSET", std::addressof(handle), sizeof(handle), /*cond = */ 0)
        ? ImVec4{0, 1, 0, 1}
        : ImVec4{1, 1, 1, 1};
      ImGui::TextColored(tint, "texture[%lu]", handle.id());
      Preview(*element, {100.0F, 100.0F});
      ImGui::EndDragDropSource();
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right) and !ImGui::IsPopupOpen("texture_menu"))
    {
      ImGui::OpenPopup("texture_menu");
      self->selected_texture = handle;
    }
    static constexpr auto kPopUpFlags = ImGuiWindowFlags_None;
    if (ImGui::BeginPopup("texture_menu", kPopUpFlags))
    {
      ImGui::Text("texure[%lu]", self->selected_texture.id());
      ImGui::Separator();
      if (ImGui::Button("delete"))
      {
        delete_this_texture = self->selected_texture;
        ImGui::CloseCurrentPopup();
      }
      Visit(element, ImGuiFieldFormatter{});
      ImGui::EndPopup();
    }
    ImGui::PopID();
  }
  ImGui::End();

  if (delete_this_texture)
  {
    resources.remove(*delete_this_texture);
  }

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(texture_viewer);
