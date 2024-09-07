// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"

using namespace sde;
using namespace sde::game;


struct scene_viewer
{};

bool load(scene_viewer* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  return true;
}


bool save(scene_viewer* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  return true;
}


bool initialize(scene_viewer* self, sde::game::Assets& assets, const sde::AppProperties& app) { return true; }

void scene_hierarchy(SceneHandle root, const sde::game::Assets& assets)
{
  const auto scene_ref = assets.scenes.get_if(root);
  if (!scene_ref)
  {
    return;
  }
  if (ImGui::TreeNode(scene_ref->name.c_str()))
  {
    for (const auto& script_handle : scene_ref->children)
    {
      scene_hierarchy(script_handle, assets);
    }
    ImGui::TreePop();
  }
}

bool update(scene_viewer* self, sde::game::Assets& assets, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return false;
  }

  ImGui::Begin("scenes");
  for (const auto& [handle, scene] : assets.scenes)
  {
    scene_hierarchy(handle, assets);
  }
  ImGui::End();

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(scene_viewer);