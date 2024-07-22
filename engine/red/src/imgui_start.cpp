// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

// RED
#include "red/imgui_start.hpp"

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

class ImGuiStart final : public ScriptRuntime
{
public:
  ImGuiStart() : ScriptRuntime{"ImGuiStart"} {}

  ~ImGuiStart() {}

private:
  asset::path imgui_ini_path_ = {};
  ImGuiContext* imgui_context_managed_;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> named{"imgui_ini_path", imgui_ini_path_};
    return true;
  }

  bool onSave(OArchive& ar, const SharedAssets& assets) const override
  {
    using namespace sde::serial;
    ar << named{"imgui_ini_path", imgui_ini_path_};
    ImGui::SaveIniSettingsToDisk(imgui_ini_path_.string().c_str());
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    IMGUI_CHECKVERSION();

    imgui_context_managed_ = ImGui::CreateContext();

    ImGui::SetCurrentContext(imgui_context_managed_);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(app.window), true);
    ImGui_ImplOpenGL3_Init(kGLSLVersion);

    return true;
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (!assets->contains<ImGuiContext*>())
    {
      assets->emplace<ImGuiContext*>(imgui_context_managed_);
    }
    ImGui::SetCurrentContext(imgui_context_managed_);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    app_state.enabled = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);

    return {};
  }
};

std::unique_ptr<ScriptRuntime> _ImGuiStart() { return std::make_unique<ImGuiStart>(); }
