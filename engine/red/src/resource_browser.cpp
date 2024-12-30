#define SDE_SCRIPT_TYPE_NAME "resource_browser"

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


struct resource_browser : native_script_data
{};

template <typename ArchiveT> bool serialize(resource_browser* self, ArchiveT& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(resource_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool shutdown(resource_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool update(resource_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }
  ImGui::Begin(self->guid());
  Visit(resources, ImGuiFieldFormatter{});
  ImGui::End();
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(resource_browser);