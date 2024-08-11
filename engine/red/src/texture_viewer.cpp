// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// RED
#include "red/imgui_common.hpp"
#include "red/texture_viewer.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;


class TextureViewer final : public ScriptRuntime
{
public:
  TextureViewer() : ScriptRuntime{"TextureViewer"} {}

private:
  TextureHandle selected_texture_;

  bool onLoad(IArchive& ar) override
  {
    using namespace sde::serial;
    return true;
  }

  bool onSave(OArchive& ar) const override
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

    std::optional<TextureHandle> delete_this_texture;

    ImGui::Begin("textures");
    for (const auto& [handle, element] : assets.graphics.textures)
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
        selected_texture_ = handle;
      }
      static constexpr auto kPopUpFlags = ImGuiWindowFlags_None;
      if (ImGui::BeginPopup("texture_menu", kPopUpFlags))
      {
        ImGui::Text("texure[%lu]", selected_texture_.id());
        ImGui::Separator();
        if (ImGui::Button("delete"))
        {
          delete_this_texture = selected_texture_;
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
      assets.graphics.textures.remove(*delete_this_texture);
    }
    return {};
  }
};

std::unique_ptr<ScriptRuntime> _TextureViewer() { return std::make_unique<TextureViewer>(); }
