// C++ Standard Library
#include <fstream>
#include <iomanip>
#include <ostream>
#include <string_view>

// JSON
#include <nlohmann/json.hpp>

// SDE
#include "sde/game/assets.hpp"
#include "sde/game/scene_manifest.hpp"
#include "sde/logging.hpp"

namespace sde::game
{
namespace
{

template <typename ResourceT>
bool ensure(const nlohmann::json& object, const char* name, const Resource<ResourceT>& resource)
{
  return IterateUntil(resource, [name, &object](const auto& field) {
    if (object.count(field.name))
    {
      return true;
    }

    {
      SDE_LOG_ERROR_FMT("SceneManifestError::kInvalidLoadJSONLayout (missing %s::%s)", name, field.name);
      return false;
    }
  });
}

template <typename T, typename LoadElement>
bool loadArray(sde::vector<T>& elements, const nlohmann::json& json_array, LoadElement load_element)
{
  if (!json_array.is_array())
  {
    return false;
  }
  elements.reserve(json_array.size());
  for (const auto& element_json : json_array)
  {
    elements.emplace_back();
    if (!load_element(elements.back(), element_json))
    {
      return false;
    }
  }
  return true;
}

template <typename T, typename SaveElement>
bool saveArray(const sde::vector<T>& elements, nlohmann::json& json_array, SaveElement save_element)
{
  auto out_json_array = nlohmann::json::array();
  for (const auto& element : elements)
  {
    nlohmann::json element_json;
    if (save_element(element, element_json))
    {
      out_json_array.push_back(std::move(element_json));
    }
    else
    {
      return false;
    }
  }
  json_array = std::move(out_json_array);
  return true;
}

}  // namespace

std::ostream& operator<<(std::ostream& os, SceneManifestError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(SceneManifestError::kInvalidLoadJSONPath)
    SDE_OS_ENUM_CASE(SceneManifestError::kInvalidLoadJSONLayout)
    SDE_OS_ENUM_CASE(SceneManifestError::kInvalidSaveJSONPath)
    SDE_OS_ENUM_CASE(SceneManifestError::kFailedSaveJSON)
    SDE_OS_ENUM_CASE(SceneManifestError::kRootNotInSceneGraph)
    SDE_OS_ENUM_CASE(SceneManifestError::kSceneAlreadyInGraph)
  }
  return os;
}

template <> expected<void, SceneManifestError> SceneScriptData::load(const nlohmann::json& json)
{
  if (!ensure(json, "SceneScriptData", *this))
  {
    return make_unexpected(SceneManifestError::kInvalidLoadJSONLayout);
  }

  // Script library path
  {
    const auto path_json = json["path"];
    this->path = static_cast<const std::string>(path_json);
  }

  // Script data path
  if (const auto data_json = json["data"]; data_json.is_string())
  {
    this->data.emplace(static_cast<const std::string>(data_json));
  }
  else
  {
    SDE_ASSERT_TRUE(data_json.is_null());
  }

  return {};
}

template <> expected<void, SceneManifestError> SceneManifestEntry::load(const nlohmann::json& json)
{
  if (!ensure(json, "SceneManifestEntry", *this))
  {
    return make_unexpected(SceneManifestError::kInvalidLoadJSONLayout);
  }

  if (!loadArray(this->pre_scripts, json["pre_scripts"], [](auto& element, const auto& json) -> bool {
        return element.load(json).has_value();
      }))
  {
    return make_unexpected(SceneManifestError::kInvalidLoadJSONLayout);
  }

  if (!loadArray(this->post_scripts, json["post_scripts"], [](auto& element, const auto& json) {
        return element.load(json).has_value();
      }))
  {
    return make_unexpected(SceneManifestError::kInvalidLoadJSONLayout);
  }

  if (!loadArray(this->children, json["children"], [](auto& element, const auto& json) -> bool {
        if (json.is_string())
        {
          element = static_cast<const sde::string>(json);
          return true;
        }
        return false;
      }))
  {
    return make_unexpected(SceneManifestError::kInvalidLoadJSONLayout);
  }

  return {};
}

template <> expected<void, SceneManifestError> SceneManifest::load(const nlohmann::json& json)
{
  if (!ensure(json, "SceneManifest", *this))
  {
    return make_unexpected(SceneManifestError::kInvalidLoadJSONLayout);
  }

  this->root_ = static_cast<std::string>(json["root"]);

  for (const auto& [scene_key_, scene_json] : json["scenes"].items())
  {
    const sde::string scene_key{scene_key_};
    auto [scene_data_itr, added] = this->scenes_.try_emplace(scene_key);
    if (!added)
    {
      SDE_LOG_ERROR_FMT("SceneManifestError::kInvalidLoadJSONPath (duplicate key: %s)", scene_key.c_str());
      return make_unexpected(SceneManifestError::kInvalidLoadJSONPath);
    }

    auto scene_data_or_error = scene_data_itr->second.load(scene_json);
    if (!scene_data_or_error.has_value())
    {
      return make_unexpected(scene_data_or_error.error());
    }
  }

  if (this->scenes_.count(this->root_) == 0)
  {
    SDE_LOG_ERROR_FMT("SceneManifestError::kRootNotInSceneGraph (root: %s)", this->root_.c_str());
    return make_unexpected(SceneManifestError::kRootNotInSceneGraph);
  }
  return {};
}

expected<void, SceneManifestError> SceneManifest::load(const asset::path& path)
{
  if (std::ifstream ifs{path}; ifs.is_open())
  {
    nlohmann::json manifest_json;
    ifs >> manifest_json;
    return this->load(manifest_json);
  }
  SDE_LOG_ERROR_FMT("SceneManifestError::kInvalidLoadJSONPath (cannot find: %s)", path.c_str());
  return make_unexpected(SceneManifestError::kInvalidLoadJSONPath);
}

template <> expected<void, SceneManifestError> SceneScriptData::save(nlohmann::json& json) const
{
  json["path"] = this->path.string();
  if (this->data.has_value())
  {
    json["data"] = this->data->string();
  }
  else
  {
    json["data"] = nlohmann::json(nullptr);
  }
  return {};
}

template <> expected<void, SceneManifestError> SceneManifestEntry::save(nlohmann::json& json) const
{
  if (!saveArray(
        this->pre_scripts, json["pre_scripts"], [](const SceneScriptData& script, nlohmann::json& json) -> bool {
          return script.save(json).has_value();
        }))
  {
    return make_unexpected(SceneManifestError::kFailedSaveJSON);
  }
  if (!saveArray(
        this->post_scripts, json["post_scripts"], [](const SceneScriptData& script, nlohmann::json& json) -> bool {
          return script.save(json).has_value();
        }))
  {
    return make_unexpected(SceneManifestError::kFailedSaveJSON);
  }
  if (!saveArray(this->children, json["children"], [](const sde::string& scene_name, nlohmann::json& json) -> bool {
        json = std::string{scene_name};
        return true;
      }))
  {
    return make_unexpected(SceneManifestError::kFailedSaveJSON);
  }
  return {};
}

template <> expected<void, SceneManifestError> SceneManifest::save(nlohmann::json& json) const
{
  nlohmann::json scenes_json;
  for (const auto& [scene_key, scene_data] : this->scenes_)
  {
    if (auto ok_or_error = scene_data.save(scenes_json[std::string{scene_key}]); !ok_or_error.has_value())
    {
      return make_unexpected(ok_or_error.error());
    }
  }
  json["scenes"] = std::move(scenes_json);
  json["root"] = this->root_;
  return {};
}

expected<void, SceneManifestError> SceneManifest::save(const asset::path& path) const
{
  nlohmann::json manifest_json;
  if (auto ok_or_error = this->save(manifest_json); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  if (std::ofstream ofs{path}; ofs.is_open())
  {
    ofs << std::setfill(' ') << std::setw(4) << manifest_json;
    return {};
  }

  SDE_LOG_ERROR_FMT("SceneManifestError::kInvalidSaveJSONPath (cannot find: %s)", path.c_str());
  return make_unexpected(SceneManifestError::kInvalidSaveJSONPath);
}

expected<SceneManifest, SceneManifestError> SceneManifest::create(const asset::path& path)
{
  SceneManifest manifest;

  if (auto ok_or_error = manifest.load(path); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return manifest;
}

void SceneManifest::setRoot(const sde::string& scene_name) { root_ = scene_name; }

expected<void, SceneManifestError> SceneManifest::setScene(const sde::string& scene_name, SceneManifestEntry&& entry)
{
  if (auto [itr, added] = scenes_.try_emplace(scene_name); added)
  {
    itr->second = std::move(entry);
    return {};
  }
  SDE_LOG_ERROR() << scene_name << " previously added";
  return make_unexpected(SceneManifestError::kSceneAlreadyInGraph);
}

}  // namespace sde::game
