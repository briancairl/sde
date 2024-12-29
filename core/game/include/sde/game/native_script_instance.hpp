/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_instance.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <optional>
#include <string_view>

// SDE
#include "sde/asset.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/game_resources_fwd.hpp"
#include "sde/game/library.hpp"
#include "sde/game/library_fwd.hpp"
#include "sde/game/library_handle.hpp"
#include "sde/game/native_script_base.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/native_script_runtime_fwd.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"
#include "sde/time.hpp"

namespace sde::game
{
/**
 * @brief A weak reference to a native script's "update" method and associated instance data
 *
 * This is meant to be called in topologically sorted sequence of game updates
 */
struct NativeScriptInstanceRef
{
public:
  NativeScriptInstanceRef(const NativeScriptInstanceRef& other) = default;
  NativeScriptInstanceRef& operator=(const NativeScriptInstanceRef& other) = default;

  NativeScriptInstanceRef(NativeScriptInstanceRef&& other) = default;
  NativeScriptInstanceRef& operator=(NativeScriptInstanceRef&& other) = default;

  ~NativeScriptInstanceRef() = default;

  NativeScriptInstanceRef(dl::Function<bool(void*, void*, const void*)> call, void* data);

  bool call(GameResources& resources, const AppProperties& app) const
  {
    return call_(data_, reinterpret_cast<void*>(&resources), reinterpret_cast<const void*>(&app));
  }

  bool operator()(GameResources& resources, const AppProperties& app) const { return this->call(resources, app); }

private:
  /// Update/Initialize call
  dl::Function<bool(void*, void*, const void*)> call_;

  /// Weak reference to instance data
  void* data_ = nullptr;
};

class NativeScriptInstance : Resource<NativeScriptInstance>
{
  friend fundemental_type;

public:
  explicit NativeScriptInstance(NativeScriptMethods methods);

  NativeScriptInstance() = default;
  ~NativeScriptInstance();

  NativeScriptInstance(NativeScriptInstance&& other);
  NativeScriptInstance& operator=(NativeScriptInstance&& other);

  NativeScriptInstance(const NativeScriptInstance& other) = default;
  NativeScriptInstance& operator=(const NativeScriptInstance& other) = default;

  constexpr bool isValid() const { return methods_.isValid() and (data_ != nullptr); }

  void reset(NativeScriptMethods methods);

  void reset();

  void swap(NativeScriptInstance& other);

  bool initialize(NativeScriptInstanceHandle handle, GameResources& resources, const AppProperties& app) const;

  bool load(IArchive& iar) const;

  bool save(OArchive& oar) const;

  std::string_view name() const { return methods_.on_get_name(); }

  script_version_t version() const { return methods_.on_get_version(); }

  asset::path path() const;

  NativeScriptInstanceRef get() const;

  NativeScriptInstanceRef operator*() const { return this->get(); }

private:
  /// Instance methods
  NativeScriptMethods methods_ = {};

  /// Instance data
  mutable void* data_ = nullptr;

  auto field_list() { return FieldList(Field{"methods", methods_}, _Stub{"data", data_}); }
};


struct NativeScriptInstanceData : Resource<NativeScriptInstanceData>
{
  /// Name of script instance
  sde::string name = {};

  /// Script from which this instance is based
  NativeScriptHandle instance_parent;

  /// Path to script instance data
  std::optional<asset::path> instance_data_path = std::nullopt;

  /// Actual script instance
  NativeScriptInstance instance = {};

  /// Returns true if script data was loaded from disk
  bool isDataFromDisk() const
  {
    return instance.isValid() and instance_data_path.has_value() and data_version == instance.version();
  }

  // clang-format off
  auto field_list()
  {
    return FieldList(
      Field{"name", name},
      Field{"instance_parent", instance_parent},
      Field{"instance_data_path", instance_data_path},
      Field{"instance", instance},
    );
  }
  // clang-format on
};

enum class NativeScriptInstanceError
{
  kInvalidHandle,
  kElementAlreadyExists,
  kNativeScriptInvalid,
  kInstanceLoadNotPossible,
  kInstanceLoadFailed,
  kInstanceSaveFailed,
};

std::ostream& operator<<(std::ostream& os, NativeScriptInstanceError error);


class NativeScriptInstanceCache : public ResourceCache<NativeScriptInstanceCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  NativeScriptInstanceHandle to_handle(const sde::string& name) const;

  /**
   * @brief Loads data for a particular script instance
   */
  expected<void, NativeScriptInstanceError>
  load(NativeScriptInstanceHandle handle, const asset::path& script_data_path);

  /**
   * @brief Saves data for a particular script instance
   */
  expected<void, NativeScriptInstanceError>
  save(NativeScriptInstanceHandle handle, const asset::path& script_data_path) const;

private:
  sde::unordered_map<sde::string, NativeScriptInstanceHandle> name_to_instance_lookup_;
  expected<void, NativeScriptInstanceError> reload(dependencies deps, NativeScriptInstanceData& library);
  expected<void, NativeScriptInstanceError> unload(dependencies deps, NativeScriptInstanceData& library);
  expected<NativeScriptInstanceData, NativeScriptInstanceError>
  generate(dependencies deps, sde::string name, const NativeScriptHandle& instance_parent);
  void when_created(dependencies deps, NativeScriptInstanceHandle handle, const NativeScriptInstanceData* data);
  void when_removed(dependencies deps, NativeScriptInstanceHandle handle, NativeScriptInstanceData* data);
};

}  // namespace sde::game
