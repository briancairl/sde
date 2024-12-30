#define SDE_SCRIPT_TYPE_NAME "script_browser"

// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"

using namespace sde;
using namespace sde::game;


struct script_browser : native_script_data
{};

template <typename ArchiveT> bool serialize(script_browser* self, ArchiveT& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(script_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool shutdown(script_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

bool update(script_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }

  ImGui::Begin("scripts");

  static constexpr auto kTableCols = 2;
  static constexpr auto kTableFlags =
    ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders;
  if (ImGui::BeginTable("scripts", kTableCols, kTableFlags))
  {
    for (const auto& [handle, script] : resources.get<NativeScriptCache>())
    {
      ImGui::TableNextColumn();
      ImGui::Text("%d", static_cast<int>(handle.id()));
      ImGui::TableNextColumn();
      ImGui::Text("%s", script->name.c_str());
    }
    ImGui::EndTable();
    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_LIBRARY_PAYLOAD"))
      {
        SDE_ASSERT_EQ(payload->DataSize, sizeof(LibraryHandle));
        const LibraryHandle library_handle{*reinterpret_cast<const LibraryHandle*>(payload->Data)};
        const auto script_or_error = resources.create<NativeScriptCache>(library_handle);
        (void)script_or_error;
      }
      ImGui::EndDragDropTarget();
    }
  }

  ImGui::End();

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(script_browser);