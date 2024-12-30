#define SDE_SCRIPT_TYPE_NAME "drag_and_drop_loader"

// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/native_script_runtime.hpp"

using namespace sde;
using namespace sde::game;


struct drag_and_drop_loader : native_script_data
{};

template <typename ArchiveT> bool serialize(drag_and_drop_loader* self, ArchiveT& ar) { return true; }

bool initialize(drag_and_drop_loader* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool shutdown(drag_and_drop_loader* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  return true;
}

bool update(drag_and_drop_loader* self, sde::game::GameResources& resources, const sde::AppProperties& app)
{
  for (const auto& payload : app.drag_and_drop_payloads)
  {
    if (payload.path.extension() == ".so")
    {
      if (auto ok_or_error = resources.create<LibraryCache>(payload.path); !ok_or_error.has_value())
      {
        SDE_LOG_ERROR_FMT("Failed to load: %s", payload.path.string().c_str());
      }
    }
    else if (payload.path.extension() == ".wav")
    {
      if (auto ok_or_error = resources.create<audio::SoundCache>(payload.path); !ok_or_error.has_value())
      {
        SDE_LOG_ERROR_FMT("Failed to load: %s", payload.path.string().c_str());
      }
    }
    else if (const auto ext = payload.path.extension(); (ext == ".png") or (ext == ".jpg") or (ext == ".jpeg"))
    {
      if (auto ok_or_error = resources.create<graphics::TextureCache>(payload.path); !ok_or_error.has_value())
      {
        SDE_LOG_ERROR_FMT("Failed to load: %s", payload.path.string().c_str());
      }
    }
    else
    {
      SDE_LOG_WARN_FMT("File has unrecognized extension: %s", payload.path.string().c_str());
    }
  }
  return true;
}


SDE_NATIVE_SCRIPT__REGISTER_AUTO(drag_and_drop_loader);