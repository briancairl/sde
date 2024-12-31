#define SDE_SCRIPT_TYPE_NAME "entity_browser"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"


// RED
#include "red/components/common.hpp"


using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

struct entity_browser : native_script_data
{};

template <typename ArchiveT> bool serialize(entity_browser* self, ArchiveT& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(entity_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool shutdown(entity_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

bool update(entity_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }
  ImGui::Begin("entities");

  static constexpr auto kTableCols = 3;
  static constexpr auto kTableFlags =
    ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders;

  if (ImGui::BeginTable("entities", kTableCols, kTableFlags))
  {
    EntityHandle remove_next = {};
    for (const auto& [handle, entity] : resources.get<EntityCache>())
    {
      ImGui::PushID(handle.id());
      ImGui::TableNextColumn();
      if (ImGui::Button("x"))
      {
        remove_next = handle;
      }
      ImGui::TableNextColumn();
      ImGui::Text("%d", static_cast<int>(handle.id()));
      ImGui::TableNextColumn();
      for (const auto& component : entity->components)
      {
        if (const auto c = resources(component))
        {
          ImGui::Text("%s", c->name.c_str());
        }
      }
      ImGui::PopID();
    }
    if (remove_next)
    {
      resources.remove(remove_next);
    }
    ImGui::EndTable();
  }

  ImGui::End();
  return true;
}

SDE_NATIVE_SCRIPT__REGISTER_AUTO(entity_browser);