/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_instance.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>

// SDE
#include "sde/app_fwd.hpp"
#include "sde/asset.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/native_script_fwd.hpp"
#include "sde/game/native_script_handle.hpp"
#include "sde/game/native_script_instance_fwd.hpp"
#include "sde/game/native_script_instance_handle.hpp"
#include "sde/game/native_script_methods.hpp"
#include "sde/game/native_script_typedefs.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"
#include "sde/time.hpp"
#include "sde/unordered_map.hpp"

namespace sde::game
{

class GameResources;

class NativeScriptInstance : public Resource<NativeScriptInstance>
{
  friend fundemental_type;
  friend class NativeScriptInstanceCache;

public:
  explicit NativeScriptInstance(NativeScriptMethods methods);

  ~NativeScriptInstance();
  NativeScriptInstance() = default;

  NativeScriptInstance(NativeScriptInstance&& other);
  NativeScriptInstance& operator=(NativeScriptInstance&& other);

  bool isValid() const { return methods_.isValid() and (data_ != nullptr); }

  void swap(NativeScriptInstance& other);

  bool load(IArchiveAssociative& iar) const;

  bool save(OArchiveAssociative& oar) const;

  bool load(const asset::path& path) const;

  bool save(const asset::path& path) const;

  std::string_view type() const { return methods_.on_get_type_name(); }

  script_version_t version() const { return methods_.on_get_version(); }

  bool initialize(
    NativeScriptInstanceHandle handle,
    std::string_view name,
    GameResources& resources,
    const AppProperties& app_properties) const;

  bool update(GameResources& resources, const AppProperties& app_properties) const;

  bool shutdown(GameResources& resources, const AppProperties& app_properties) const;

private:
  NativeScriptInstance(const NativeScriptInstance& other) = delete;
  NativeScriptInstance& operator=(const NativeScriptInstance& other) = delete;

  void reset();

  void reset(NativeScriptMethods methods);

  /// Instance methods
  NativeScriptMethods methods_ = {};

  /// Instance data
  mutable void* data_ = nullptr;

  auto field_list() { return FieldList(Field{"methods", methods_}); }
};


struct NativeScriptInstanceData : Resource<NativeScriptInstanceData>
{
  /// Name of script instance
  sde::string name = {};

  /// Script from which this instance is based
  NativeScriptHandle parent;

  /// Actual script instance
  NativeScriptInstance instance = {};

  // clang-format off
  auto field_list()
  {
    return FieldList(
      Field{"name", name},
      Field{"parent", parent},
      Field{"instance", instance}
    );
  }
  // clang-format on
};

enum class NativeScriptInstanceError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kNativeScriptInvalid,
  kInstanceDataUnavailable,
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

private:
  sde::unordered_map<sde::string, NativeScriptInstanceHandle> name_to_instance_lookup_;
  expected<void, NativeScriptInstanceError> reload(dependencies deps, NativeScriptInstanceData& library);
  expected<void, NativeScriptInstanceError> unload(dependencies deps, NativeScriptInstanceData& library);
  expected<NativeScriptInstanceData, NativeScriptInstanceError>
  generate(dependencies deps, sde::string name, const NativeScriptHandle& parent);
  bool when_created(dependencies deps, NativeScriptInstanceHandle handle, const NativeScriptInstanceData* data);
  bool when_removed(dependencies deps, NativeScriptInstanceHandle handle, NativeScriptInstanceData* data);
};

}  // namespace sde::game
