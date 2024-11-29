#define SDE_SCRIPT_NAME "imgui_end"

// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/native_script_runtime.hpp"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

using namespace sde;
using namespace sde::game;


struct imgui_end
{};


bool load(imgui_end* self, sde::game::IArchive& ar) { return true; }


bool save(imgui_end* self, sde::game::OArchive& ar) { return true; }


bool initialize(imgui_end* self, sde::game::Assets& assets, const sde::AppProperties& app) { return true; }

bool update(imgui_end* self, sde::game::Assets& assets, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return false;
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(imgui_end);