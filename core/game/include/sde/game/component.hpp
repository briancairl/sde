/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>
#include <string_view>

// EnTT
#include <entt/fwd.hpp>

// SDE
#include "sde/asset.hpp"
#include "sde/dl/library.hpp"
#include "sde/game/archive_fwd.hpp"
#include "sde/game/component_fwd.hpp"
#include "sde/game/component_handle.hpp"
#include "sde/game/library_fwd.hpp"
#include "sde/game/library_handle.hpp"
#include "sde/game/registry.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/string.hpp"
#include "sde/type_name.hpp"
#include "sde/unordered_map.hpp"

namespace sde::game
{

enum class ComponentError
{
  SDE_RESOURCE_CACHE_ERROR_ENUMS,
  kComponentLibraryInvalid,
  kComponentLibraryMissingFunction,
  kComponentAlreadyLoaded
};

std::ostream& operator<<(std::ostream& os, ComponentError error);

class ComponentIO : public Resource<ComponentIO>
{
  friend fundemental_type;

public:
  ComponentIO() = default;
  ~ComponentIO();

  ComponentIO(ComponentIO&& other);
  ComponentIO& operator=(ComponentIO&& other);

  void swap(ComponentIO& other);

  void reset();
  bool reset(const sde::string& name, const dl::Library& library);

  operator bool() const
  {
    return IterateUntil(*this, [](auto& field) -> bool { return field.get(); });
  }

  std::string_view name() const { return name_(); }
  void load(IArchiveAssociative& ar, EntityID id, Registry& registry) const;
  void save(OArchiveAssociative& ar, EntityID id, const Registry& registry) const;

private:
  ComponentIO(const ComponentIO&) = delete;
  ComponentIO& operator=(const ComponentIO&) = delete;

  dl::Function<const char*(void)> name_;
  dl::Function<void(void*, void*, void*)> on_load_;
  dl::Function<void(void*, void*, const void*)> on_save_;

  auto field_list()
  {
    // clang-format off
    return FieldList(_Stub{"name", name_}, _Stub{"on_load", on_load_}, _Stub{"on_save", on_save_});
    // clang-format on
  }
};

struct ComponentData : Resource<ComponentData>
{
  LibraryHandle library;
  sde::string name;
  ComponentIO io;

  auto field_list() { return FieldList(Field{"library", library}, Field{"name", name}, _Stub{"io", io}); }
};

class ComponentCache : public ResourceCache<ComponentCache>
{
  friend fundemental_type;

public:
  using fundemental_type::to_handle;
  ComponentHandle to_handle(const sde::string& name) const;

private:
  sde::unordered_map<sde::string, ComponentHandle> type_name_to_component_handle_lookup_;
  expected<void, ComponentError> reload(dependencies dep, ComponentData& library);
  expected<void, ComponentError> unload(dependencies dep, ComponentData& library);
  expected<ComponentData, ComponentError> generate(dependencies dep, const sde::string& name, const asset::path& path);
  expected<ComponentData, ComponentError> generate(dependencies dep, const sde::string& name, LibraryHandle library);
  bool when_created(dependencies dep, ComponentHandle handle, const ComponentData* data);
  bool when_removed(dependencies dep, ComponentHandle handle, const ComponentData* data);
};

}  // namespace sde::game
