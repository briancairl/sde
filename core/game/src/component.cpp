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
  std::swap(this->on_load_, other.on_load_);
  std::swap(this->on_save_, other.on_save_);
}

void ComponentIO::reset()
{
  on_load_.reset();
  on_save_.reset();
}

bool ComponentIO::reset(const dl::Library& library)
{
  return IterateUntil(*this, [&](auto& field) {
    auto symbol_or_error = library.get(field.name);
    if (!symbol_or_error.has_value())
    {
      SDE_LOG_ERROR_FMT("ComponentIO.%s : %s", field.name, symbol_or_error.error().details);
      return false;
    }
    else
    {
      (*field) = std::move(symbol_or_error).value();
    }
    return true;
  });
}

bool ComponentIO::load(IArchive& ar, entt::entity id, entt::registry& registry) const
{
  SDE_ASSERT_TRUE(on_load_);
  return on_load_(reinterpret_cast<void*>(&ar), reinterpret_cast<void*>(&id), reinterpret_cast<void*>(&registry));
}

bool ComponentIO::save(OArchive& ar, entt::entity id, const entt::registry& registry) const
{
  SDE_ASSERT_TRUE(on_save_);
  return on_save_(reinterpret_cast<void*>(&ar), reinterpret_cast<void*>(&id), reinterpret_cast<const void*>(&registry));
}


ComponentCache::ComponentCache(LibraryCache& libraries) : libraries_{std::addressof(libraries)} {}

expected<void, ComponentError> ComponentCache::reload(ComponentData& component)
{
  auto library_ptr = libraries_->get_if(component.library);
  if (library_ptr == nullptr)
  {
    SDE_LOG_ERROR() << "ComponentLibraryInvalid: " << SDE_OSNV(component.library);
    return make_unexpected(ComponentError::kComponentLibraryInvalid);
  }

  if (!component.io.reset(library_ptr->lib))
  {
    SDE_LOG_ERROR() << "ComponentLibraryMissingFunction: " << SDE_OSNV(component.library);
    return make_unexpected(ComponentError::kComponentLibraryMissingFunction);
  }

  component.name = component.io.name();

  return {};
}

expected<void, ComponentError> ComponentCache::unload(ComponentData& component)
{
  component.io.reset();
  return {};
}

expected<ComponentData, ComponentError> ComponentCache::generate(LibraryHandle library)
{
  ComponentData data{.library = library};

  auto ok_or_error = reload(data);

  if (const auto itr = type_name_to_component_data_lookup_.find(data.name);
      itr != std::end(type_name_to_component_data_lookup_))
  {
    return make_unexpected(ComponentError::kComponentAlreadyLoaded);
  }

  if (!ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

expected<ComponentData, ComponentError> ComponentCache::generate(const asset::path& path)
{
  auto library_or_error = libraries_->create(path);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR() << "ComponentLibraryInvalid: " << SDE_OSNV(path) << ", " << SDE_OSNV(library_or_error.error());
    return make_unexpected(ComponentError::kComponentLibraryInvalid);
  }
  return this->generate(library_or_error->handle);
}

void ComponentCache::when_created(ComponentHandle handle, const ComponentData* data)
{
  type_name_to_component_data_lookup_.emplace(data->name, data);
  type_name_to_component_handle_lookup_.emplace(data->name, handle);
}

void ComponentCache::when_removed(ComponentHandle handle, const ComponentData* data)
{
  type_name_to_component_data_lookup_.erase(data->name);
  type_name_to_component_handle_lookup_.erase(data->name);
}

const ComponentData* ComponentCache::get_if(const std::string& name) const
{
  const auto itr = type_name_to_component_data_lookup_.find(name);
  if (itr == std::end(type_name_to_component_data_lookup_))
  {
    return nullptr;
  }
  return itr->second;
}

const ComponentHandle ComponentCache::get_handle(const std::string& name) const
{
  const auto itr = type_name_to_component_handle_lookup_.find(name);
  if (itr == std::end(type_name_to_component_handle_lookup_))
  {
    return ComponentHandle::null();
  }
  return itr->second;
}

}  // namespace sde::game