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
#include "red/texture_viewer.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;


struct ImGuiFieldFormatter
{
  template <typename T> bool operator()(std::size_t depth, const BasicField<T>& field)
  {
    if (depth > 0)
    {
      ImGui::Dummy(ImVec2(depth * 10, 0.0));
      ImGui::SameLine();
    }

    using U = std::remove_const_t<T>;
    if constexpr (std::is_same_v<U, asset::path>)
    {
      ImGui::Text("%s : %s", field.name, field->string().c_str());
    }
    else if constexpr (std::is_same_v<U, Vec2i>)
    {
      ImGui::Text("%s : (%d x %d)", field.name, field->x(), field->y());
    }
    else if constexpr (std::is_same_v<U, Hash>)
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
    else if constexpr (is_resource_v<T>)
    {
      return ImGui::CollapsingHeader(format("%s : ...", field.name));
    }
    else
    {
      ImGui::Text("%s : ...", field.name);
    }
    return true;
  }
};


void TexturePreviewImage(const Texture& texture, const ImVec2& tile_size)
{
  const ImColor border_color{ImGui::GetStyle().Colors[ImGuiCol_Border]};
  auto* drawlist = ImGui::GetWindowDrawList();
  const auto pos = ImGui::GetCursorScreenPos();
  ImGui::Dummy(tile_size);

  drawlist->AddRect(pos, pos + tile_size, border_color);
  if (texture.shape.width() > texture.shape.height())
  {
    const float aspect = static_cast<float>(texture.shape.height()) / static_cast<float>(texture.shape.width());
    const ImVec2 display_size{tile_size.x, tile_size.x * aspect};
    const ImVec2 centering{(tile_size - display_size) * 0.5F};
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
    const ImVec2 display_size{tile_size.y * aspect, tile_size.y};
    const ImVec2 centering{(tile_size - display_size) * 0.5F};
    drawlist->AddImage(
      reinterpret_cast<void*>(texture.native_id.value()),
      pos + centering,
      pos + centering + display_size,
      ImVec2{0, 0},
      ImVec2{1, 1},
      IM_COL32_WHITE);
  }
  if (ImGui::IsMouseHoveringRect(pos, pos + tile_size))
  {
    drawlist->AddRectFilled(pos, pos + tile_size, ImColor{1.0F, 1.0F, 0.0F, 0.2F});
  }
}


class TextureViewer final : public ScriptRuntime
{
public:
  TextureViewer() : ScriptRuntime{"TextureViewer"} {}

private:
  TextureHandle selected_texture_;

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

    std::optional<TextureHandle> delete_this_texture;

    ImGui::Begin("textures");
    for (const auto& [handle, element] : assets.graphics.textures)
    {
      if (element->source_image.isNull())
      {
        continue;
      }
      ImGui::PushID(handle.id());

      const auto max_size = ImGui::GetWindowContentRegionMax() - ImGui::GetWindowContentRegionMin();
      TexturePreviewImage(*element, {max_size.x, max_size.x});

      if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
      {
        const ImVec4 tint =
          ImGui::SetDragDropPayload("SDE_TEXTURE_ASSET", std::addressof(handle), sizeof(handle), /*cond = */ 0)
          ? ImVec4{0, 1, 0, 1}
          : ImVec4{1, 1, 1, 1};
        ImGui::TextColored(tint, "texture[%lu]", handle.id());
        TexturePreviewImage(*element, {100.0F, 100.0F});
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
