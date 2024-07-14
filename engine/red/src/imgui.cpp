// C++ Standard Library
#include <ostream>

// SDE
#include "sde/asset.hpp"
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

// RED
#include "red/imgui.hpp"

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

class ImGuiWrapper final : public ScriptRuntime
{
public:
  ImGuiWrapper() : ScriptRuntime{"ImGui"} {}

  ~ImGuiWrapper()
  {
    if (imgui_context_ == nullptr)
    {
      return;
    }
    ImGui::DestroyContext(imgui_context_);
  }

private:
  ImGuiContext* imgui_context_ = nullptr;
  asset::path imgui_ini_path_ = {};
  bool imgui_overlay_enabled_ = false;

  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar >> named{"imgui_overlay_enabled", imgui_overlay_enabled_};
    ar >> named{"imgui_ini_path", imgui_ini_path_};
    return true;
  }

  bool onSave(OArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    ar << named{"imgui_overlay_enabled", imgui_overlay_enabled_};
    ar << named{"imgui_ini_path", imgui_ini_path_};
    ImGui::SaveIniSettingsToDisk(imgui_ini_path_.string().c_str());
    return true;
  }

  bool onInitialize(Systems& systems, SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    IMGUI_CHECKVERSION();

    if (imgui_context_ = ImGui::CreateContext(); imgui_context_ == nullptr)
    {
      return false;
    }

    if (asset::exists(imgui_ini_path_))
    {
      // ImGui::LoadIniSettingsFromDisk(imgui_ini_path_.string().c_str());
    }
    else
    {
      imgui_ini_path_ = "/tmp/imgui.ini";
      imgui_overlay_enabled_ = true;

      // Setup style
      ImGui::StyleColorsDark();
    }

    ImGui::SetCurrentContext(imgui_context_);
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(app.window), true);
    ImGui_ImplOpenGL3_Init(kGLSLVersion);

    return true;
  }

  expected<void, ScriptError>
  onUpdate(Systems& systems, SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::BeginMainMenuBar();
    ImGui::Checkbox("imgui_overlay_enabled", &imgui_overlay_enabled_);
    ImGui::EndMainMenuBar();

    if (imgui_overlay_enabled_)
    {
      ImGui::Begin("sounds");
      app_state.enabled = !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
      for (const auto& [handle, element] : assets.audio.sound_data)
      {
        ImGui::Text("%lu : %s", handle.id(), element.value.path.string().c_str());
      }
      ImGui::End();

      ImGui::Begin("textures");
      app_state.enabled = !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
      for (const auto& [handle, element] : assets.graphics.textures)
      {
        ImGui::Text("%lu", handle.id());
        ImGui::Image(
          reinterpret_cast<void*>(element->native_id.value()), ImVec2(element->shape.width(), element->shape.height()));
      }
      ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return {};
  }
};

std::unique_ptr<sde::game::ScriptRuntime> createImGui() { return std::make_unique<ImGuiWrapper>(); }
