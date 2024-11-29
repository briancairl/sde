/**
 * @copyright 2024-present Brian Cairl
 *
 * @file component.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

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
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/type_name.hpp"
#include "sde/unordered_map.hpp"

namespace sde::game
{

enum class ComponentError
{
  kInvalidHandle,
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
  bool reset(const dl::Library& library);

  constexpr operator bool() const { return on_load_ and on_save_; }

  const char* name() const { return name_(); }
  bool load(IArchive& ar, entt::entity id, entt::registry& registry) const;
  bool save(OArchive& ar, entt::entity id, const entt::registry& registry) const;

private:
  ComponentIO(const ComponentIO&) = delete;
  ComponentIO& operator=(const ComponentIO&) = delete;

  dl::Function<const char*(void)> name_;
  dl::Function<bool(void*, void*, void*)> on_load_;
  dl::Function<bool(void*, void*, const void*)> on_save_;

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
  std::string name;
  ComponentIO io;

  auto field_list() { return FieldList(Field{"library", library}, Field{"name", name}, _Stub{"io", io}); }
};

}  // namespace sde::game

namespace sde
{

template <> struct ResourceCacheTypes<game::ComponentCache>
{
  using error_type = game::ComponentError;
  using handle_type = game::ComponentHandle;
  using value_type = game::ComponentData;
};

}  // namespace sde

namespace sde::game
{

class ComponentCache : public ResourceCache<ComponentCache>
{
  friend fundemental_type;

public:
  explicit ComponentCache(LibraryCache& libraries);

  using fundemental_type::get_if;

  const ComponentHandle get_handle(const std::string& name) const;

  const ComponentData* get_if(const std::string& name) const;

private:
  LibraryCache* libraries_;
  sde::unordered_map<std::string, ComponentHandle> type_name_to_component_handle_lookup_;
  sde::unordered_map<std::string, const ComponentData*> type_name_to_component_data_lookup_;
  expected<void, ComponentError> reload(ComponentData& library);
  expected<void, ComponentError> unload(ComponentData& library);
  expected<ComponentData, ComponentError> generate(const asset::path& path);
  expected<ComponentData, ComponentError> generate(LibraryHandle library);
  void when_created(ComponentHandle handle, const ComponentData* data);
  void when_removed(ComponentHandle handle, const ComponentData* data);
};

}  // namespace sde::game
