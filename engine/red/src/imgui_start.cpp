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

// Decide GL+GLSL versions
#if __APPLE__
// GL 3.2 + GLSL 150
constexpr const char* kGLSLVersion{"#version 150"};
#else
// GL 3.0 + GLSL 130
constexpr const char* kGLSLVersion{"#version 130"};  // 3.0+ only
#endif


struct imgui_start
{
  asset::path imgui_ini_path = {};
  ImGuiContext* imgui_context = nullptr;
};


bool load(imgui_start* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  ar >> named{"imgui_ini_path", self->imgui_ini_path};
  return true;
}


bool save(imgui_start* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  ar << named{"imgui_ini_path", self->imgui_ini_path};
  ImGui::SaveIniSettingsToDisk(self->imgui_ini_path.string().c_str());
  return true;
}


bool initialize(imgui_start* self, sde::game::Assets& assets, const sde::AppProperties& app)
{
  IMGUI_CHECKVERSION();

  self->imgui_context = ImGui::CreateContext();

  ImGui::SetCurrentContext(self->imgui_context);
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(app.window), true);
  ImGui_ImplOpenGL3_Init(kGLSLVersion);

  SDE_LOG_INFO("ImGui initialized");
  return true;
}


bool update(imgui_start* self, sde::game::Assets& assets, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return false;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

  // ImGui::ShowMetricsWindow();
  // app_state.enabled = !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(imgui_start);