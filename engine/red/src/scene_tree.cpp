#define SDE_SCRIPT_TYPE_NAME "scene_tree"

// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"

using namespace sde;
using namespace sde::game;


struct scene_viewer : native_script_data
{};

template <typename ArchiveT> bool serialize(scene_viewer* self, ArchiveT& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(scene_viewer* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }


bool shutdown(scene_viewer* self, sde::game::GameResources& resources, const sde::AppProperties& app) { return true; }

void scene_hierarchy(SceneHandle handle, sde::game::GameResources& resources)
{
  const auto scene_ref = resources(handle);
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
        resources.update_if_exists(handle, [child_handle](auto& v) { v.nodes.push_back({.child = child_handle}); });
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
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.8F, 0.8F, 0.8F, 1.0F});
    const bool open = ImGui::TreeNode("nodes");
    ImGui::PopStyleColor();
    if (open)
    {
      for (const auto& node : scene_ref->nodes)
      {
        if (node.child)
        {
          scene_hierarchy(node.child, resources);
        }
        else if (!node.script)
        {
          continue;
        }
        else if (const auto script = resources(node.script))
        {
          ImGui::Text(
            "%s (type:%s, ver:%lu)", script->name.c_str(), script->instance.type().data(), script->instance.version());
        }
      }
      ImGui::TreePop();
    };
    ImGui::TreePop();
  }
  ImGui::PopID();
}

bool update(scene_viewer* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }

  ImGui::Begin("scenes");
  if (ImGui::SmallButton("new scene"))
  {
    // if (const auto ok_or_error = resources.create<SceneCache>(sde::string{"unamed"}); !ok_or_error.has_value())
    // {
    //   SDE_LOG_ERROR() << "Failed to create new scene: " << ok_or_error.error();
    // }
  }
  for (const auto& [handle, scene] : resources.get<SceneCache>())
  {
    scene_hierarchy(handle, resources);
  }
  ImGui::End();

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(scene_viewer);