// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/library.hpp"
#include "sde/game/native_script.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

NativeScript::~NativeScript() { this->reset(); }

NativeScript::NativeScript(NativeScript&& other) { this->swap(other); }

NativeScript& NativeScript::operator=(NativeScript&& other)
{
  this->swap(other);
  return *this;
}

void NativeScript::swap(NativeScript& other)
{
  std::swap(this->initialized_, other.initialized_);
  std::swap(this->instance_, other.instance_);
  std::swap(this->on_create_, other.on_create_);
  std::swap(this->on_destroy_, other.on_destroy_);
  std::swap(this->on_load_, other.on_load_);
  std::swap(this->on_save_, other.on_save_);
  std::swap(this->on_initialize_, other.on_initialize_);
  std::swap(this->on_update_, other.on_update_);
}

void NativeScript::reset()
{
  if (instance_ == nullptr)
  {
    return;
  }
  else
  {
    on_destroy_(std::free, instance_);
    SDE_LOG_ERROR_FMT("NativeScript[%p] destroyed", instance_);
    instance_ = nullptr;
    initialized_ = false;
    on_create_.reset();
    on_destroy_.reset();
    on_load_.reset();
    on_save_.reset();
    on_initialize_.reset();
    on_update_.reset();
  }
}

bool NativeScript::reset(const dl::Library& library)
{
  if (!IterateUntil(*this, [&library](auto& field) {
        auto symbol_or_error = library.get(field.name);
        if (!symbol_or_error.has_value())
        {
          SDE_LOG_ERROR_FMT("NativeScript.%s : %s", field.name, symbol_or_error.error().details);
          return false;
        }
        else
        {
          (*field) = std::move(symbol_or_error).value();
        }
        return true;
      }))
  {
    return false;
  }
  else if (instance_ = on_create_(std::malloc); instance_ == nullptr)
  {
    return false;
  }
  return true;
}

bool NativeScript::load(IArchive& ar) const
{
  SDE_ASSERT_TRUE(on_load_);
  return on_load_(reinterpret_cast<void*>(&ar));
}

bool NativeScript::save(OArchive& ar) const
{
  SDE_ASSERT_TRUE(on_save_);
  return on_save_(reinterpret_cast<void*>(&ar));
}

expected<void, NativeScriptCallError> NativeScript::call(Assets& assets, const AppProperties& app_properties) const
{
  SDE_ASSERT_TRUE(on_initialize_);
  SDE_ASSERT_TRUE(on_update_);

  // run initialization routine, if not already run successfully
  initialized_ = initialized_ or
    on_initialize_(reinterpret_cast<void*>(instance_),
                   reinterpret_cast<void*>(&assets),
                   reinterpret_cast<const void*>(&app_properties));

  // abort if initialization failed
  if (!initialized_)
  {
    return make_unexpected(NativeScriptCallError::kNotInitialized);
  }

  // run update behavior
  if (on_update_(
        reinterpret_cast<void*>(instance_),
        reinterpret_cast<void*>(&assets),
        reinterpret_cast<const void*>(&app_properties)))
  {
    return {};
  }
  return make_unexpected(NativeScriptCallError::kNotUpdated);
}


NativeScriptCache::NativeScriptCache(LibraryCache& libraries) : libraries_{std::addressof(libraries)} {}

expected<void, NativeScriptError> NativeScriptCache::reload(NativeScriptData& script)
{
  const auto* library_ptr = libraries_->get_if(script.library);

  if (library_ptr == nullptr)
  {
    SDE_LOG_ERROR("NativeScriptError::kScriptLibraryInvalid");
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }

  if (!script.script.reset(library_ptr->lib))
  {
    SDE_LOG_ERROR("NativeScriptError::kScriptLibraryMissingFunction");
    return make_unexpected(NativeScriptError::kScriptLibraryMissingFunction);
  }

  script.name = script.script.name();

  return {};
}

expected<void, NativeScriptError> NativeScriptCache::unload(NativeScriptData& script)
{
  script.script.reset();
  return {};
}

expected<NativeScriptData, NativeScriptError> NativeScriptCache::generate(LibraryHandle library)
{
  NativeScriptData data{.library = library};

  if (const auto ok_or_error = reload(data); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

expected<NativeScriptData, NativeScriptError>
NativeScriptCache::generate(const asset::path& path, const LibraryFlags& flags)
{
  auto library_or_error = libraries_->create(path, flags);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR("NativeScriptError::kScriptLibraryInvalid");
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }
  return this->generate(library_or_error->handle);
}

}  // namespace sde::game