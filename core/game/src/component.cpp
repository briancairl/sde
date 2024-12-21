// C++ Standard Library
#include <ostream>

// EnTT
#include <entt/entt.hpp>

// SDE
#include "sde/game/archive.hpp"
#include "sde/game/component.hpp"
#include "sde/game/library.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, ComponentError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(ComponentError::kInvalidHandle)
    SDE_OS_ENUM_CASE(ComponentError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(ComponentError::kComponentLibraryInvalid)
    SDE_OS_ENUM_CASE(ComponentError::kComponentLibraryMissingFunction)
    SDE_OS_ENUM_CASE(ComponentError::kComponentAlreadyLoaded)
  }
  return os;
}

ComponentIO::~ComponentIO() { this->reset(); }

ComponentIO::ComponentIO(ComponentIO&& other) { this->swap(other); }

ComponentIO& ComponentIO::operator=(ComponentIO&& other)
{
  this->swap(other);
  return *this;
}

void ComponentIO::swap(ComponentIO& other)
{
  std::swap(this->name_, other.name_);
  std::swap(this->on_load_, other.on_load_);
  std::swap(this->on_save_, other.on_save_);
}

void ComponentIO::reset()
{
  IterateUntil(*this, [](auto& field) {
    field->reset();
    return true;
  });
}

bool ComponentIO::reset(const sde::string& name, const dl::Library& library)
{
  return IterateUntil(*this, [&](auto& field) {
    const sde::string field_name = (name + "_" + field.name);
    auto symbol_or_error = library.get(field_name.c_str());
    if (!symbol_or_error.has_value())
    {
      SDE_LOG_ERROR() << "ComponentIO : " << field_name << " : " << symbol_or_error.error();
      return false;
    }
    else
    {
      (*field) = std::move(symbol_or_error).value();
    }
    return true;
  });
}

void ComponentIO::load(IArchive& ar, EntityID id, Registry& registry) const
{
  SDE_ASSERT_TRUE(on_load_);
  on_load_(reinterpret_cast<void*>(&ar), reinterpret_cast<void*>(&id), reinterpret_cast<void*>(&registry));
}

void ComponentIO::save(OArchive& ar, EntityID id, const Registry& registry) const
{
  SDE_ASSERT_TRUE(on_save_);
  on_save_(reinterpret_cast<void*>(&ar), reinterpret_cast<void*>(&id), reinterpret_cast<const void*>(&registry));
}

expected<void, ComponentError> ComponentCache::reload(dependencies dep, ComponentData& component)
{
  auto library_ptr = dep.get<LibraryCache>().get_if(component.library);
  if (library_ptr == nullptr)
  {
    SDE_LOG_ERROR() << "ComponentLibraryInvalid: " << SDE_OSNV(component.library);
    return make_unexpected(ComponentError::kComponentLibraryInvalid);
  }

  if (!component.io.reset(component.name, library_ptr->lib))
  {
    SDE_LOG_ERROR() << "ComponentLibraryMissingFunction: " << SDE_OSNV(component.library) << " "
                    << SDE_OSNV(library_ptr->path);
    return make_unexpected(ComponentError::kComponentLibraryMissingFunction);
  }

  return {};
}

expected<void, ComponentError> ComponentCache::unload(dependencies dep, ComponentData& component)
{
  component.io.reset();
  return {};
}

expected<ComponentData, ComponentError>
ComponentCache::generate(dependencies dep, const sde::string& name, LibraryHandle library)
{
  ComponentData data{.library = library, .name = name};

  auto ok_or_error = reload(dep, data);

  const auto itr = type_name_to_component_handle_lookup_.find(data.name);
  if (itr != std::end(type_name_to_component_handle_lookup_))
  {
    return make_unexpected(ComponentError::kComponentAlreadyLoaded);
  }

  if (!ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

expected<ComponentData, ComponentError>
ComponentCache::generate(dependencies dep, const sde::string& name, const asset::path& path)
{
  auto library_or_error = dep.get<LibraryCache>().find_or_create(path, path);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR() << "ComponentLibraryInvalid: " << SDE_OSNV(path) << ", " << SDE_OSNV(library_or_error.error());
    return make_unexpected(ComponentError::kComponentLibraryInvalid);
  }
  return this->generate(dep, name, library_or_error->handle);
}

void ComponentCache::when_created(ComponentHandle handle, const ComponentData* data)
{
  type_name_to_component_handle_lookup_.emplace(data->name, handle);
}

void ComponentCache::when_removed(ComponentHandle handle, const ComponentData* data)
{
  type_name_to_component_handle_lookup_.erase(data->name);
}

ComponentHandle ComponentCache::to_handle(const sde::string& name) const
{
  const auto itr = type_name_to_component_handle_lookup_.find(name);
  if (itr == std::end(type_name_to_component_handle_lookup_))
  {
    return ComponentHandle::null();
  }
  return itr->second;
}

}  // namespace sde::game