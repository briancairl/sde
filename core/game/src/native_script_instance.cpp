// C++ Standard Library
#include <ostream>

// SDE
#include "sde/format.hpp"
#include "sde/game/archive.hpp"
#include "sde/game/native_script.hpp"
#include "sde/game/native_script_header.hpp"
#include "sde/game/native_script_instance.hpp"
#include "sde/logging.hpp"

namespace sde::game
{
namespace
{

asset::path
getPath(const NativeScriptInstanceHandle handle, const sde::string& name, const NativeScriptInstance& instance)
{
  return {sde::format("%s_%lu_%s_%lu.bin", name.c_str(), handle.id(), instance.type().data(), instance.version())};
}

}  // namespace

std::ostream& operator<<(std::ostream& os, NativeScriptInstanceError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASES_FOR_RESOURCE_CACHE_ERRORS(NativeScriptInstanceError)
    SDE_OS_ENUM_CASE(NativeScriptInstanceError::kNativeScriptInvalid)
    SDE_OS_ENUM_CASE(NativeScriptInstanceError::kInstanceDataUnavailable)
    SDE_OS_ENUM_CASE(NativeScriptInstanceError::kInstanceLoadFailed)
    SDE_OS_ENUM_CASE(NativeScriptInstanceError::kInstanceSaveFailed)
  }
  return os;
}

NativeScriptInstance::NativeScriptInstance(NativeScriptMethods methods) : methods_{std::move(methods)}, data_{nullptr}
{}

NativeScriptInstance::NativeScriptInstance(NativeScriptInstance&& other) { this->swap(other); }

NativeScriptInstance& NativeScriptInstance::operator=(NativeScriptInstance&& other)
{
  this->swap(other);
  return *this;
}

void NativeScriptInstance::swap(NativeScriptInstance& other)
{
  std::swap(other.data_, this->data_);
  std::swap(other.methods_, this->methods_);
}

void NativeScriptInstance::reset(NativeScriptMethods methods)
{
  SDE_ASSERT_TRUE(methods.isValid()) << "attempting to reset a NativeScriptInstance with invalid methods";

  // Clear previous instance data
  this->reset();
  SDE_ASSERT_EQ(data_, nullptr);

  // Set new methods
  methods_ = std::move(methods);

  // Create new instance data
  data_ = methods_.on_create(std::malloc);
  SDE_ASSERT_NE(data_, nullptr);
}

void NativeScriptInstance::reset()
{
  if (data_ == nullptr)
  {
    return;
  }

  SDE_ASSERT_TRUE(methods_.on_destroy);
  methods_.on_destroy(std::free, data_);
  data_ = nullptr;
}

bool NativeScriptInstance::initialize(
  NativeScriptInstanceHandle handle,
  std::string_view name,
  GameResources& resources,
  const AppProperties& app_properties) const
{
  SDE_ASSERT_NE(data_, nullptr);
  SDE_ASSERT_TRUE(methods_.on_get_version);

  auto* basic_data = reinterpret_cast<native_script_header*>(data_);

  // Check if already initialized
  if (!basic_data->name.empty())
  {
    SDE_LOG_DEBUG() << "Previously initialized: " << SDE_OSNV(basic_data->uid) << SDE_OSNV(basic_data->name)
                    << SDE_OSNV(basic_data->version);
    return true;
  }

  // Set basic info
  {
    basic_data->uid = handle.id();
    basic_data->name = name;
    basic_data->version = methods_.on_get_version();
    SDE_LOG_DEBUG() << "Initialized: " << SDE_OSNV(basic_data->uid) << SDE_OSNV(basic_data->name)
                    << SDE_OSNV(basic_data->version);
  }

  SDE_ASSERT_TRUE(methods_.on_initialize);
  return methods_.on_initialize(
    data_, reinterpret_cast<void*>(&resources), reinterpret_cast<const void*>(&app_properties));
}

bool NativeScriptInstance::update(GameResources& resources, const AppProperties& app_properties) const
{
  SDE_ASSERT_NE(data_, nullptr);
  SDE_ASSERT_TRUE(methods_.on_update);
  SDE_ASSERT_FALSE(reinterpret_cast<native_script_header*>(data_)->name.empty()) << "script not initialized";
  return methods_.on_update(data_, reinterpret_cast<void*>(&resources), reinterpret_cast<const void*>(&app_properties));
}

bool NativeScriptInstance::shutdown(GameResources& resources, const AppProperties& app_properties) const
{
  SDE_ASSERT_NE(data_, nullptr);
  SDE_ASSERT_TRUE(methods_.on_shutdown);

  auto* basic_data = reinterpret_cast<native_script_header*>(data_);
  SDE_ASSERT_FALSE(basic_data->name.empty()) << "script not initialized";

  if (methods_.on_shutdown(data_, reinterpret_cast<void*>(&resources), reinterpret_cast<const void*>(&app_properties)))
  {
    basic_data->uid = 0;
    basic_data->name = {};
    basic_data->version = 0;
    return true;
  }
  return false;
}


bool NativeScriptInstance::load(IArchiveAssociative& iar) const
{
  SDE_ASSERT_NE(data_, nullptr);
  SDE_ASSERT_TRUE(methods_.on_load);

  // Load script header first
  auto& header = *reinterpret_cast<native_script_header*>(data_);
  {
    iar >> ::sde::serial::named{"__header__", header};
  }

  // If version matches, load script data
  if (const auto current_version = methods_.on_get_version() != header.version)
  {
    SDE_LOG_WARN() << "Not loading script data because version has changed from " << SDE_OSNV(header.version) << " to "
                   << SDE_OSNV(current_version);
    header.version = current_version;
  }
  return methods_.on_load(data_, reinterpret_cast<void*>(&iar));
}

bool NativeScriptInstance::save(OArchiveAssociative& oar) const
{
  SDE_ASSERT_NE(data_, nullptr);
  SDE_ASSERT_TRUE(methods_.on_save);

  // Save script header first
  const auto& header = *reinterpret_cast<const native_script_header*>(data_);
  {
    oar << ::sde::serial::named{"__header__", header};
  }

  // Save actual script data
  return methods_.on_save(data_, reinterpret_cast<void*>(&oar));
}

NativeScriptInstanceHandle NativeScriptInstanceCache::to_handle(const sde::string& name) const
{
  const auto itr = name_to_instance_lookup_.find(name);
  if (itr == std::end(name_to_instance_lookup_))
  {
    return NativeScriptInstanceHandle::null();
  }
  return itr->second;
}

void NativeScriptInstanceCache::when_created(
  [[maybe_unused]] dependencies deps,
  NativeScriptInstanceHandle handle,
  const NativeScriptInstanceData* data)
{
  const auto [itr, added] = name_to_instance_lookup_.emplace(data->name, handle);
  SDE_ASSERT_TRUE(added) << "Script " << data->name << " was already added as " << itr->first;
}

void NativeScriptInstanceCache::when_removed(
  [[maybe_unused]] dependencies deps,
  NativeScriptInstanceHandle handle,
  NativeScriptInstanceData* data)
{
  name_to_instance_lookup_.erase(data->name);
}

expected<void, NativeScriptInstanceError>
NativeScriptInstanceCache::load(NativeScriptInstanceHandle handle, const asset::path& script_data_path)
{
  // TODO(enhancement) move this to NativeScriptInstance

  // Get script data by handle
  auto itr = handle_to_value_cache_.find(handle);
  if (itr == std::end(handle_to_value_cache_))
  {
    return make_unexpected(NativeScriptInstanceError::kInvalidHandle);
  }

  auto& script = itr->second.value;

  const asset::path path = script_data_path / getPath(handle, script.name, script.instance);

  // Stop here if path does not exist
  if (!asset::exists(path))
  {
    return make_unexpected(NativeScriptInstanceError::kInstanceDataUnavailable);
  }

  // Open script instance data file
  auto ifs_or_error = serial::file_istream::create(path);
  if (!ifs_or_error.has_value())
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << " " << ifs_or_error.error();
    return make_unexpected(NativeScriptInstanceError::kInstanceLoadFailed);
  }

  // Wrap file stream in archive interface
  auto iar_basic = serial::binary_ifarchive{*ifs_or_error};

  // Create associative wrapper for archive
  auto iar_or_error = serial::make_associative(iar_basic);
  if (!iar_or_error.has_value())
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << " " << iar_or_error.error();
    return make_unexpected(NativeScriptInstanceError::kInstanceLoadFailed);
  }

  // Load data for this instance
  if (!script.instance.load(*iar_or_error))
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << ": load routine failed";
    return make_unexpected(NativeScriptInstanceError::kInstanceLoadFailed);
  }

  return {};
}

expected<void, NativeScriptInstanceError>
NativeScriptInstanceCache::save(NativeScriptInstanceHandle handle, const asset::path& script_data_path) const
{
  // TODO(enhancement) move this to NativeScriptInstance

  // Get script data by handle
  auto itr = handle_to_value_cache_.find(handle);
  if (itr == std::end(handle_to_value_cache_))
  {
    return make_unexpected(NativeScriptInstanceError::kInvalidHandle);
  }

  auto& script = itr->second.value;

  const asset::path path = script_data_path / getPath(handle, script.name, script.instance);

  // Open script instance data file
  auto ofs_or_error = serial::file_ostream::create(path);
  if (!ofs_or_error.has_value())
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << " " << ofs_or_error.error();
    return make_unexpected(NativeScriptInstanceError::kInstanceSaveFailed);
  }

  // Wrap file stream in archive interface
  auto oar_basic = serial::binary_ofarchive{*ofs_or_error};

  // Create associative wrapper for archive
  auto oar_or_error = serial::make_associative(oar_basic);
  if (!oar_or_error.has_value())
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << " " << oar_or_error.error();
    return make_unexpected(NativeScriptInstanceError::kInstanceSaveFailed);
  }

  // Write data for this instance
  if (!script.instance.save(*oar_or_error))
  {
    SDE_LOG_ERROR() << SDE_OSNV(path) << ": save routine failed";
    return make_unexpected(NativeScriptInstanceError::kInstanceSaveFailed);
  }

  return {};
}

expected<void, NativeScriptInstanceError>
NativeScriptInstanceCache::reload(dependencies deps, NativeScriptInstanceData& script)
{
  // Get source script to instance
  auto native_script_or_error = deps(script.parent);
  if (!native_script_or_error)
  {
    SDE_LOG_ERROR() << SDE_OSNV(script.parent) << " is an invalid NativeScript handle";
    return make_unexpected(NativeScriptInstanceError::kNativeScriptInvalid);
  }

  // Create an instance of source script
  script.instance.reset(native_script_or_error->methods);

  return {};
}

expected<void, NativeScriptInstanceError>
NativeScriptInstanceCache::unload(dependencies deps, NativeScriptInstanceData& script)
{
  if (script.instance.isValid())
  {
    script.instance.reset();
    SDE_LOG_INFO() << SDE_OSNV(script.name) << " (instance of " << SDE_OSNV(script.parent) << ") was reset";
  }
  else
  {
    SDE_LOG_WARN() << SDE_OSNV(script.name) << " (instance of " << SDE_OSNV(script.parent) << ") not active";
  }
  return {};
}

expected<NativeScriptInstanceData, NativeScriptInstanceError>
NativeScriptInstanceCache::generate(dependencies deps, sde::string name, const NativeScriptHandle& parent)
{
  NativeScriptInstanceData data;
  {
    data.name = std::move(name);
    data.parent = parent;
  }

  if (auto ok_or_error = reload(deps, data); !ok_or_error.has_value())
  {
    SDE_LOG_ERROR() << ok_or_error.error();
    return make_unexpected(ok_or_error.error());
  }

  return {std::move(data)};
}

}  // namespace sde::game