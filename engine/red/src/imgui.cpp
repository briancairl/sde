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

struct ImGuiFieldFormatter
{
  template <typename T> void operator()(std::size_t depth, const BasicField<T>& field)
  {
    ImGui::Dummy(ImVec2(depth * 10, 0.0));
    ImGui::SameLine();

    using U = std::remove_const_t<T>;
    if constexpr (is_resource_cache_v<T>)
    {
      ImGui::Text("%s : ...", field.name);
      for (const auto& [handle, element] : field.get())
      {
        Visit(element, ImGuiFieldFormatter{}, depth + 1);
      }
    }
    if constexpr (std::is_same_v<U, asset::path>)
    {
      ImGui::Text("%s : %s", field.name, field->string().c_str());
    }
    else if constexpr (std::is_same_v<U, Vec2i>)
    {
      ImGui::Text("%s : (%d x %d)", field.name, field->x(), field->y());
    }
    else if constexpr (std::is_same_v<U, Hash>)
    {
      ImGui::Text("%s : {%lu}", field.name, field->value);
    }
    else if constexpr (std::is_enum_v<U>)
    {
      ImGui::Text("%s : %d", field.name, static_cast<int>(field.get()));
    }
    else if constexpr (std::is_integral_v<U>)
    {
      ImGui::Text("%s : %d", field.name, static_cast<int>(field.get()));
    }
    else
    {
      ImGui::Text("%s : ...", field.name);
    }
  }
};

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
      ImGui::Begin("asset-tree");
      app_state.enabled = !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
      Visit(assets, ImGuiFieldFormatter{});
      ImGui::End();

      ImGui::Begin("sounds");
      app_state.enabled = !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
      for (const auto& [handle, element] : assets.audio.sound_data)
      {
        ImGui::Text("Sound[%lu]", handle.id());
        {
          static constexpr bool kChildShowBoarders = true;
          static constexpr auto kChildFlags = ImGuiWindowFlags_None;
          ImGui::PushID(handle.id());
          ImGui::BeginChild("#Sound", ImVec2{0.0F, 200.0F}, kChildShowBoarders, kChildFlags);
          Visit(element, ImGuiFieldFormatter{});
          ImGui::EndChild();
          ImGui::PopID();
        }
      }
      ImGui::End();

      ImGui::Begin("textures");
      app_state.enabled = !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
      for (const auto& [handle, element] : assets.graphics.textures)
      {
        ImGui::Text("Texture[%lu]", handle.id());
        {
          static constexpr bool kChildShowBoarders = true;
          static constexpr auto kChildFlags = ImGuiWindowFlags_None;
          ImGui::PushID(handle.id());
          ImGui::BeginChild("#Texture", ImVec2{0.0F, 200.0F}, kChildShowBoarders, kChildFlags);
          Visit(element, ImGuiFieldFormatter{});
          ImGui::EndChild();
          ImGui::PopID();
        }
        if (ImGui::IsItemHovered())
        {
          if (ImGui::BeginTooltip())
          {
            static constexpr float kTextureWidth = 400.0F;
            ImGui::Image(
              reinterpret_cast<void*>(element->native_id.value()),
              ImVec2{kTextureWidth * element->shape.aspect(), kTextureWidth});
            ImGui::EndTooltip();
          }
        }
      }
      ImGui::End();

      ImGui::Begin("tile_sets");
      app_state.enabled = !ImGui::IsWindowHovered(ImGuiFocusedFlags_AnyWindow);
      for (const auto& [handle, element] : assets.graphics.tile_sets)
      {
        ImGui::Text("TileSet[%lu] from (%lu)", handle.id(), element->tile_atlas.id());
        auto atlas_texture = assets.graphics.textures(element->tile_atlas);
        for (const auto& bounds : element->tile_bounds)
        {
          static constexpr float kTileWidth = 100.0F;
          ImGui::Image(
            reinterpret_cast<void*>(atlas_texture->native_id.value()),
            ImVec2{kTileWidth, kTileWidth},
            ImVec2{bounds.max().x(), bounds.max().y()},
            ImVec2{bounds.min().x(), bounds.min().y()});
          ImGui::SameLine();
        }
        ImGui::NewLine();
      }
      ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return {};
  }
};

std::unique_ptr<ScriptRuntime> createImGui() { return std::make_unique<ImGuiWrapper>(); }
