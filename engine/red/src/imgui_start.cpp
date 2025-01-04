#define SDE_SCRIPT_TYPE_NAME "imgui_start"

// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/native_script_runtime.hpp"
#include "sde/graphics/colors.hpp"
#include "sde/graphics/render_target.hpp"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

// Decide GL+GLSL versions
#if __APPLE__
// GL 3.2 + GLSL 150
constexpr const char* kGLSLVersion{"#version 150"};
#else
// GL 3.0 + GLSL 130
constexpr const char* kGLSLVersion{"#version 130"};  // 3.0+ only
#endif


struct imgui_start : native_script_data
{
  RenderTargetHandle render_target = RenderTargetHandle::null();
  asset::path imgui_ini_path = {};
  ImGuiContext* imgui_context = nullptr;
};


template <typename ArchiveT> bool serialize(imgui_start* self, ArchiveT& ar)
{
  using namespace sde::serial;
  ar& named{"render_target", self->render_target};
  ar& named{"imgui_ini_path", self->imgui_ini_path};
  return true;
}

bool shutdown(imgui_start* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (self->imgui_context)
  {
    ImGui::SaveIniSettingsToDisk(self->imgui_ini_path.string().c_str());
    ImGui::DestroyContext(self->imgui_context);
    ImGui::SetCurrentContext(nullptr);
    self->imgui_context = nullptr;
  }
  return true;
}

bool initialize(imgui_start* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (!resources.assign(self->render_target))
  {
    SDE_LOG_ERROR() << "Missing sprite shader";
    return false;
  }

  IMGUI_CHECKVERSION();

  self->imgui_context = ImGui::CreateContext();

  ImGui::SetCurrentContext(self->imgui_context);
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(app.window), true);
  ImGui_ImplOpenGL3_Init(kGLSLVersion);

  if (asset::exists(self->imgui_ini_path))
  {
    SDE_ASSERT_NE(self->imgui_context, nullptr);
    ImGui::LoadIniSettingsFromDisk(self->imgui_ini_path.string().c_str());
  }
  else
  {
    self->imgui_ini_path = resources.path("imgui.ini");
  }

  SDE_LOG_INFO() << "ImGui initialized";
  return true;
}


bool update(imgui_start* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return false;
  }

  if (auto render_target = resources(self->render_target); render_target)
  {
    render_target->reset(Black());
  }
  else
  {
    return false;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

  ImGui::ShowMetricsWindow();
  // app_state.enabled = !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(imgui_start);