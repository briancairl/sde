#define SDE_SCRIPT_NAME "entity_browser"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"


// RED
#include "red/components.hpp"


using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

struct entity_browser
{};

bool load(entity_browser* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  return true;
}

bool save(entity_browser* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(entity_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool update(entity_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }
  ImGui::Begin("entities");

  static constexpr auto kTableCols = 2;
  static constexpr auto kTableFlags =
    ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders;

  if (ImGui::BeginTable("entities", kTableCols, kTableFlags))
  {
    for (const auto& [handle, entity] : resources.get<EntityCache>())
    {
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
    }
    ImGui::EndTable();
  }

  ImGui::End();
  return true;
}

SDE_NATIVE_SCRIPT__REGISTER_AUTO(entity_browser);