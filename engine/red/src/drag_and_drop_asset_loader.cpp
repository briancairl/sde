// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/script_impl.hpp"
#include "sde/logging.hpp"

// RED
#include "red/drag_and_drop_asset_loader.hpp"

using namespace sde;
using namespace sde::game;
using namespace sde::audio;

class DragAndDropAssetLoader final : public ScriptRuntime
{
public:
  DragAndDropAssetLoader() : ScriptRuntime{"DragAndDropAssetLoader"} {}

private:
  bool onLoad(IArchive& ar, SharedAssets& assets) override
  {
    using namespace sde::serial;
    return true;
  }

  bool onSave(OArchive& ar, const SharedAssets& assets) const override
  {
    using namespace sde::serial;
    return true;
  }

  bool onInitialize(SharedAssets& assets, AppState& app_state, const AppProperties& app) override { return true; }

  expected<void, ScriptError> onUpdate(SharedAssets& assets, AppState& app_state, const AppProperties& app) override
  {
    for (const auto& payload : app.drag_and_drop_payloads)
    {
      if (payload.path.extension() == ".wav")
      {
        if (auto ok_or_error = assets.audio.sounds.create(payload.path); !ok_or_error.has_value())
        {
          SDE_LOG_ERROR_FMT("Failed to load: %s", payload.path.string().c_str());
        }
      }
      else if (const auto ext = payload.path.extension(); (ext == ".png") or (ext == ".jpg") or (ext == ".jpeg"))
      {
        if (auto ok_or_error = assets.graphics.textures.create(payload.path); !ok_or_error.has_value())
        {
          SDE_LOG_ERROR_FMT("Failed to load: %s", payload.path.string().c_str());
        }
      }
      else
      {
        SDE_LOG_WARN_FMT("File has unrecognized extension: %s", payload.path.string().c_str());
      }
    }
    return {};
  }
};

std::unique_ptr<ScriptRuntime> _DragAndDropAssetLoader() { return std::make_unique<DragAndDropAssetLoader>(); }
