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
#include "red/imgui_end.hpp"

using namespace sde;
using namespace sde::game;


class ImGuiEnd final : public ScriptRuntime
{
public:
  ImGuiEnd() : ScriptRuntime{"ImGui"} {}

  ~ImGuiEnd() {}

private:
  bool onLoad(IArchive& ar) override { return true; }

  bool onSave(OArchive& ar) const override { return true; }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    IMGUI_CHECKVERSION();
    return true;
  }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    if (assets->contains<ImGuiContext*>())
    {
      ImGui::SetCurrentContext(assets->get<ImGuiContext*>());
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    return {};
  }
};

std::unique_ptr<ScriptRuntime> _ImGuiEnd() { return std::make_unique<ImGuiEnd>(); }
