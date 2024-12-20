// C++ Standard Library
#include <fstream>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/game/component.hpp"
#include "sde/game/component_preload.hpp"
#include "sde/game/game_resources.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

expected<void, ComponentError> componentPreload(GameResources& resources, const asset::path& preload_manifest_path)
{
  nlohmann::json manifest_json;
  if (std::ifstream ifs{preload_manifest_path}; ifs.is_open())
  {
    ifs >> manifest_json;
  }
  else
  {
    return make_unexpected(ComponentError::kComponentLibraryInvalid);
  }

  for (const auto& [key_json, data_json] : manifest_json.items())
  {
    const sde::string component_name{key_json};
    if (auto ok_or_error =
          resources.find_or_create<ComponentCache>(component_name, component_name, asset::path{data_json["path"]});
        !ok_or_error.has_value())
    {
      SDE_LOG_ERROR() << "Failed load: " << component_name << " : " << ok_or_error.error();
      return make_unexpected(ok_or_error.error());
    }
    else if (ok_or_error->status == ResourceStatus::kCreated)
    {
      SDE_LOG_INFO() << "Component loaded: " << component_name;
    }
    else
    {
      SDE_LOG_INFO() << "Component found: " << component_name;
    }
  }

  return {};
}

}  // namespace sde::game