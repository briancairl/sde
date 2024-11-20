#define SDE_SCRIPT_NAME "scene_tree"

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

void scene_hierarchy(SceneHandle handle, sde::game::Assets& assets)
{
  const auto scene_ref = assets.scenes.get_if(handle);
  if (!scene_ref)
  {
    return;
  }

  const bool node_open = ImGui::TreeNode(scene_ref->name.c_str());

  ImGui::PushID(scene_ref->name.c_str());
  if (ImGui::BeginDragDropTarget())
  {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SDE_SCENE_TREE_MOVE"))
    {
      SDE_ASSERT_EQ(payload->DataSize, sizeof(SceneHandle));
      if (const SceneHandle child_handle{*reinterpret_cast<const SceneHandle*>(payload->Data)}; child_handle != handle)
      {
        assets.scenes.update_if_exists(handle, [child_handle](auto& v) { v.children.push_back(child_handle); });
      }
    }
    ImGui::EndDragDropTarget();
  }

  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
  {
    const ImVec4 tint =
      ImGui::SetDragDropPayload("SDE_SCENE_TREE_MOVE", std::addressof(handle), sizeof(handle), /*cond = */ 0)
      ? ImVec4{0, 1, 0, 1}
      : ImVec4{1, 1, 1, 1};
    ImGui::TextColored(tint, "scene[%s]", scene_ref->name.c_str());
    ImGui::EndDragDropSource();
  }

  if (node_open)
  {
    if (!scene_ref->pre_scripts.empty())
    {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.8F, 0.8F, 0.8F, 1.0F});
      const bool open = ImGui::TreeNode("pre");
      ImGui::PopStyleColor();
      if (open)
      {
        for (const auto& script_handle : scene_ref->pre_scripts)
        {
          const auto script = assets.scripts.get_if(script_handle);
          ImGui::Text("%s", script->name.c_str());
        }
        ImGui::TreePop();
      };
    }

    if (!scene_ref->children.empty())
    {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.8F, 0.8F, 0.8F, 1.0F});
      const bool open = ImGui::TreeNode("children");
      ImGui::PopStyleColor();
      if (open)
      {
        for (const auto& scene_handle : scene_ref->children)
        {
          scene_hierarchy(scene_handle, assets);
        }
        ImGui::TreePop();
      };
    }

    if (!scene_ref->post_scripts.empty())
    {
      ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.8F, 0.8F, 0.8F, 1.0F});
      const bool open = ImGui::TreeNode("post");
      ImGui::PopStyleColor();
      if (open)
      {
        for (const auto& script_handle : scene_ref->post_scripts)
        {
          const auto script = assets.scripts.get_if(script_handle);
          ImGui::Text("%s", script->name.c_str());
        }
        ImGui::TreePop();
      };
    }

    ImGui::TreePop();
  }
  ImGui::PopID();
}

bool update(scene_viewer* self, sde::game::Assets& assets, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return false;
  }

  ImGui::Begin("scenes");
  if (ImGui::SmallButton("new scene"))
  {
    if (const auto ok_or_error = assets.scenes.create(SceneType::kChild, sde::string{"unamed"});
        !ok_or_error.has_value())
    {
      SDE_LOG_ERROR("failed to create new scene");
    }
  }
  for (const auto& [handle, scene] : assets.scenes)
  {
    scene_hierarchy(handle, assets);
  }
  ImGui::End();

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(scene_viewer);