// C++ Standard Library
#include <ostream>

// ImGui
#include <imgui.h>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// RED
#include "red/texture_viewer.hpp"

using namespace sde;
using namespace sde::game;


struct ImGuiFieldFormatter
{
  template <typename T> bool operator()(std::size_t depth, const BasicField<T>& field)
  {
    ImGui::Dummy(ImVec2(depth * 10, 0.0));
    ImGui::SameLine();

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


class TextureViewer final : public ScriptRuntime
{
public:
  TextureViewer() : ScriptRuntime{"TextureViewer"} {}

private:
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

    ImGui::Begin("textures");
    for (const auto& [handle, element] : assets.graphics.textures)
    {
      ImGui::PushID(handle.id());
      if (ImGui::CollapsingHeader(sde::format("texture[%lu]", handle.id())))
      {
        static constexpr bool kChildShowBoarders = true;
        static constexpr auto kChildFlags = ImGuiWindowFlags_None;
        ImGui::BeginChild(
          "#TextureProperties",
          ImVec2{0.0F, ImGui::GetTextLineHeight() * (element->field_count())},
          kChildShowBoarders,
          kChildFlags);
        Visit(element, ImGuiFieldFormatter{});
        ImGui::EndChild();
      }
      ImGui::PopID();
      if (ImGui::IsItemHovered())
      {
        if (ImGui::BeginTooltip())
        {
          static constexpr float kTextureWidth = 400.0F;
          ImGui::Image(
            reinterpret_cast<void*>(element->native_id.value()),
            ImVec2{kTextureWidth * element->shape.aspect(), kTextureWidth});
          ImGui::EndTooltip();
        }
      }
    }
    ImGui::End();
    return {};
  }
};

std::unique_ptr<ScriptRuntime> _TextureViewer() { return std::make_unique<TextureViewer>(); }
