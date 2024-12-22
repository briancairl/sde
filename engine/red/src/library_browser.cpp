#define SDE_SCRIPT_NAME "library_browser"

// C++ Standard Library
#include <ostream>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"
#include "sde/unordered_map.hpp"
#include "sde/vector.hpp"

using namespace sde;
using namespace sde::game;


struct library_browser
{
  sde::unordered_map<asset::path, sde::vector<asset::path>> search_paths = {};
};


void refresh(library_browser* self)
{
  for (auto& [search_path, library_paths] : self->search_paths)
  {
    library_paths.clear();
    for (const auto& de : asset::recursive_directory_iterator{search_path})
    {
      if (de.path().extension() == ".so")
      {
        library_paths.push_back(de.path());
      }
    }
  }
}

bool load(library_browser* self, sde::game::IArchive& ar)
{
  using namespace sde::serial;
  ar >> named{"search_paths", self->search_paths};
  return true;
}


bool save(library_browser* self, sde::game::OArchive& ar)
{
  using namespace sde::serial;
  ar << named{"search_paths", self->search_paths};
  return true;
}


bool initialize(library_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  self->search_paths.try_emplace("engine");
  refresh(self);
  return true;
}


bool update(library_browser* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }

  ImGui::Begin("libraries");
  if (ImGui::SmallButton("refresh"))
  {
    refresh(self);
  }
  for (const auto& [search_path, library_paths] : self->search_paths)
  {
    ImGui::Text("%s", search_path.string().c_str());
    static constexpr auto kTableCols = 2;
    static constexpr auto kTableFlags =
      ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders;
    if (ImGui::BeginTable(search_path.string().c_str(), kTableCols, kTableFlags))
    {
      auto& libraries = resources.get<LibraryCache>();
      for (const auto& path : library_paths)
      {
        ImGui::PushID(path.string().c_str());
        if (const auto handle = libraries.to_handle(path); handle.isNull())
        {
          ImGui::TableNextColumn();
          ImGui::Text("%s", path.string().c_str());
          ImGui::TableNextColumn();
          if (ImGui::SmallButton("load"))
          {
            [[maybe_unused]] const auto _ = libraries.create(path);
          }
        }
        else
        {
          const auto lib = libraries.get_if(handle);
          ImGui::TableNextColumn();
          ImGui::TextColored(ImVec4{0.0F, 0.8F, 0.0F, 1.0F}, "%s", path.string().c_str());

          if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
          {
            const ImVec4 tint =
              ImGui::SetDragDropPayload("SDE_LIBRARY_PAYLOAD", std::addressof(handle), sizeof(handle), /*cond = */ 0)
              ? ImVec4{0, 1, 0, 1}
              : ImVec4{1, 1, 1, 1};
            ImGui::TextColored(tint, "library[%s]", path.string().c_str());
            ImGui::EndDragDropSource();
          }

          ImGui::TableNextColumn();
          if (lib->flags.required)
          {
            ImGui::TextColored(ImVec4{0.5F, 0.5F, 0.0F, 1.0F}, "%s", "required");
          }
        }
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();

  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(library_browser);