#define SDE_SCRIPT_TYPE_NAME "logging"

// C++ Standard Library
#include <deque>
#include <fstream>
#include <string>

// ImGui
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>

// SDE
#include "sde/game/native_script_runtime.hpp"

// RED
#include "red/components/common.hpp"


using namespace sde;
using namespace sde::game;
using namespace sde::graphics;

struct logging : native_script_data
{
  std::string log_line;
  std::ofstream log_stream_wr;
  std::ifstream log_stream_rd;
  std::deque<std::string> log_feed;
  std::size_t log_line_max = 50;
  bool show_debug = true;
};

template <typename ArchiveT> bool serialize(logging* self, ArchiveT& ar)
{
  using namespace sde::serial;
  return true;
}

bool initialize(logging* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  const auto log_file_path = resources.path("logs.log");

  self->log_stream_wr.open(log_file_path, std::ios_base::app | std::ios_base::out);
  if (!self->log_stream_wr.is_open())
  {
    return false;
  }

  self->log_stream_rd.open(log_file_path);
  self->log_stream_rd.clear();
  self->log_stream_rd.seekg(0);
  if (!self->log_stream_rd.is_open())
  {
    return false;
  }

  sde::setLogStream(std::addressof(self->log_stream_wr));
  return true;
}

bool shutdown(logging* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  self->log_stream_wr.close();
  self->log_stream_rd.close();
  sde::setLogStream(nullptr);
  return true;
}

bool update(logging* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  if (ImGui::GetCurrentContext() == nullptr)
  {
    return true;
  }

  self->log_stream_rd.clear();
  while (std::getline(self->log_stream_rd, self->log_line))
  {
    self->log_feed.push_back(std::move(self->log_line));
  }
  while (self->log_feed.size() > self->log_line_max)
  {
    self->log_feed.pop_front();
  }

  ImGui::Begin(self->guid());
  {
    {
      auto n = static_cast<int>(self->log_line_max);
      if (ImGui::InputInt("max_lines", &n) and (n > 0))
      {
        self->log_line_max = static_cast<std::size_t>(n);
        SDE_LOG_WARN() << "Log feed resized to: " << self->log_line_max;
      }
      ImGui::Checkbox("debug", &self->show_debug);
    }
    ImGui::BeginChild("feed");
    {
      for (std::string_view l : self->log_feed)
      {
        if (l.find("(Debug:") != std::string_view::npos)
        {
          if (self->show_debug)
          {
            ImGui::TextColored(ImVec4{0, 1, 1, 1}, "%s", l.data());
          }
        }
        else if (l.find("(Error:") != std::string_view::npos)
        {
          ImGui::TextColored(ImVec4{1, 0, 0, 1}, "%s", l.data());
        }
        else if (l.find("(Warn:") != std::string_view::npos)
        {
          ImGui::TextColored(ImVec4{1, 1, 0, 1}, "%s", l.data());
        }
        else
        {
          ImGui::TextColored(ImVec4{1, 1, 1, 1}, "%s", l.data());
        }
      }
    }
    ImGui::EndChild();
  }
  ImGui::End();
  return true;
}

SDE_NATIVE_SCRIPT__REGISTER_AUTO(logging);