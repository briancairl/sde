// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/library.hpp"
#include "sde/game/native_script.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

std::ostream& operator<<(std::ostream& os, NativeScriptCallError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(NativeScriptCallError::kNotInitialized)
    SDE_OS_ENUM_CASE(NativeScriptCallError::kNotUpdated)
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, NativeScriptError error)
{
  switch (error)
  {
    SDE_OS_ENUM_CASE(NativeScriptError::kInvalidHandle)
    SDE_OS_ENUM_CASE(NativeScriptError::kElementAlreadyExists)
    SDE_OS_ENUM_CASE(NativeScriptError::kScriptLibraryInvalid)
    SDE_OS_ENUM_CASE(NativeScriptError::kScriptLibraryMissingFunction)
  }
  return os;
}

bool NativeScriptFn::isValid() const
{
  return IterateUntil(*this, [](const auto& fn) { return fn->isValid(); });
}

void NativeScriptFn::reset()
{
  IterateUntil(*this, [](auto& fn) {
    fn->reset();
    return true;
  });
}

template <typename ScriptT> NativeScriptBase<ScriptT>::NativeScriptBase(NativeScriptFn fn) : fn_{std::move(fn)} {}

template <typename ScriptT> void NativeScriptBase<ScriptT>::swap(NativeScriptBase<ScriptT>& other)
{
  std::swap(this->fn_, other.fn_);
}

template <typename ScriptT> bool NativeScriptBase<ScriptT>::reset(const dl::Library& library)
{
  return IterateUntil(this->fn_, [&library](auto& field) {
    auto symbol_or_error = library.get(field.name);
    if (!symbol_or_error.has_value())
    {
      SDE_LOG_ERROR_FMT("NativeScriptInstance.%s : %s", field.name, symbol_or_error.error().details);
      return false;
    }
    else
    {
      (*field) = std::move(symbol_or_error).value();
    }
    return true;
  });
}

NativeScriptInstance::NativeScriptInstance(NativeScriptFn fn) :
    NativeScriptBase{std::move(fn)}, initialized_{false}, instance_{nullptr}
{
  instance_ = fn_.on_create(std::malloc);
}

NativeScriptInstance::NativeScriptInstance(NativeScriptInstance&& other) { NativeScriptInstance::swap(other); }

NativeScriptInstance::~NativeScriptInstance() { NativeScriptInstance::reset(); }

void NativeScriptInstance::swap(NativeScriptInstance& other)
{
  Base::swap(static_cast<Base&>(other));
  std::swap(other.instance_, this->instance_);
  std::swap(other.initialized_, this->initialized_);
}

void NativeScriptInstance::reset()
{
  if (instance_ != nullptr)
  {
    fn_.on_destroy(std::free, instance_);
  }
}

bool NativeScriptInstance::load(IArchive& ar) const
{
  SDE_ASSERT_TRUE(fn_.on_load);
  return fn_.on_load(instance_, reinterpret_cast<void*>(&ar));
}

bool NativeScriptInstance::save(OArchive& ar) const
{
  SDE_ASSERT_TRUE(fn_.on_save);
  return fn_.on_save(instance_, reinterpret_cast<void*>(&ar));
}

expected<void, NativeScriptCallError>
NativeScriptInstance::initialize(Assets& assets, const AppProperties& app_properties) const
{
  // run initialization routine, if not already run successfully
  // clang-format off
  initialized_ =
    initialized_
    or
    fn_.on_initialize(reinterpret_cast<void*>(instance_),
                   reinterpret_cast<void*>(&assets),
                   reinterpret_cast<const void*>(&app_properties));

  // clang-format on
  if (initialized_)
  {
    return {};
  }
  return make_unexpected(NativeScriptCallError::kNotInitialized);
}

expected<void, NativeScriptCallError>
NativeScriptInstance::call(Assets& assets, const AppProperties& app_properties) const
{
  SDE_ASSERT_TRUE(initialized_);

  // run update behavior
  if (fn_.on_update(
        reinterpret_cast<void*>(instance_),
        reinterpret_cast<void*>(&assets),
        reinterpret_cast<const void*>(&app_properties)))
  {
    return {};
  }
  return make_unexpected(NativeScriptCallError::kNotUpdated);
}

NativeScript::NativeScript(NativeScript&& other) { NativeScript::swap(other); }

NativeScript& NativeScript::operator=(NativeScript&& other)
{
  this->swap(other);
  return *this;
}

NativeScriptInstance NativeScript::instance() const { return NativeScriptInstance{this->fn_}; }

NativeScriptCache::NativeScriptCache(LibraryCache& libraries) : libraries_{std::addressof(libraries)} {}

expected<void, NativeScriptError> NativeScriptCache::reload(NativeScriptData& script)
{
  const auto* library_ptr = libraries_->get_if(script.library);

  if (library_ptr == nullptr)
  {
    SDE_LOG_ERROR() << "ScriptLibraryInvalid: " << SDE_OSNV(script.library);
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }

  if (!script.script.reset(library_ptr->lib))
  {
    SDE_LOG_ERROR() << "ScriptLibraryMissingFunction: " << SDE_OSNV(library_ptr->lib);
    return make_unexpected(NativeScriptError::kScriptLibraryMissingFunction);
  }

  script.name = script.script.name();

  return {};
}

expected<void, NativeScriptError> NativeScriptCache::unload(NativeScriptData& script) { return {}; }

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
    SDE_LOG_ERROR() << "ScriptLibraryInvalid: " << library_or_error.error();
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }
  return this->generate(library_or_error->handle);
}

}  // namespace sde::game