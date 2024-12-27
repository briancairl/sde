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

template <typename ScriptT> std::ostream& operator<<(std::ostream& os, const NativeScriptBase<ScriptT>& script)
{
  return os << script.name() << '.' << script.version();
}

template std::ostream& operator<< <NativeScript>(std::ostream&, const NativeScriptBase<NativeScript>& script);
template std::ostream&
operator<< <NativeScriptInstance>(std::ostream&, const NativeScriptBase<NativeScriptInstance>& script);

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
  return IterateUntil(this->fn_, [this, &library](auto& field) {
    auto symbol_or_error = library.get(field.name);
    if (!symbol_or_error.has_value())
    {
      SDE_LOG_ERROR() << *this << "::" << field.name << " : " << symbol_or_error.error();
      return false;
    }
    else
    {
      (*field) = std::move(symbol_or_error).value();
    }
    return true;
  });
}

NativeScriptInstance::NativeScriptInstance(NativeScriptHandle handle, NativeScriptFn fn) :
    NativeScriptBase{std::move(fn)}, initialized_{false}, instance_{nullptr}
{
  instance_ = fn_.on_create(std::malloc);
  reinterpret_cast<native_script_data>(instance_)->uid = handle.id();
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
    SDE_ASSERT_TRUE(fn_.on_destroy);
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

NativeScriptInstance& NativeScriptInstance::operator=(NativeScriptInstance&& other)
{
  this->swap(other);
  return *this;
}

expected<void, NativeScriptCallError>
NativeScriptInstance::initialize(GameResources& resources, const AppProperties& app_properties) const
{
  SDE_ASSERT_FALSE(initialized_) << "'NativeScriptInstance::initialize' called previously";
  SDE_LOG_INFO() << "Initializing: " << fn_.on_get_name();

  // run initialization routine, if not already run successfully
  // clang-format off
  initialized_ =
    fn_.on_initialize(
      reinterpret_cast<void*>(instance_),
      reinterpret_cast<void*>(&resources),
      reinterpret_cast<const void*>(&app_properties));

  // clang-format on
  if (initialized_)
  {
    return {};
  }
  return make_unexpected(NativeScriptCallError::kNotInitialized);
}

expected<void, NativeScriptCallError>
NativeScriptInstance::call(GameResources& resources, const AppProperties& app_properties) const
{
  SDE_ASSERT_TRUE(initialized_) << "'NativeScriptInstance::initialize' not yet called";

  // run update behavior
  if (fn_.on_update(
        reinterpret_cast<void*>(instance_),
        reinterpret_cast<void*>(&resources),
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

NativeScriptInstance NativeScript::instance(script_id_t uid) const { return NativeScriptInstance{uid, this->fn_}; }

NativeScriptHandle NativeScriptCache::to_handle(const LibraryHandle& library) const
{
  const auto itr = library_to_native_script_lookup_.find(library);
  if (itr == std::end(library_to_native_script_lookup_))
  {
    return NativeScriptHandle::null();
  }
  return itr->second;
}


void NativeScriptCache::when_created(
  [[maybe_unused]] dependencies deps,
  NativeScriptHandle handle,
  const NativeScriptData* data)
{
  library_to_native_script_lookup_.emplace(data->library, handle);
}

void NativeScriptCache::when_removed(
  [[maybe_unused]] dependencies deps,
  NativeScriptHandle handle,
  const NativeScriptData* data)
{
  library_to_native_script_lookup_.erase(data->library);
}

expected<void, NativeScriptError> NativeScriptCache::reload(dependencies deps, NativeScriptData& script)
{
  const auto* library_ptr = deps.get<LibraryCache>().get_if(script.library);

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

expected<void, NativeScriptError> NativeScriptCache::unload(dependencies deps, NativeScriptData& script) { return {}; }

expected<NativeScriptData, NativeScriptError> NativeScriptCache::generate(dependencies deps, LibraryHandle library)
{
  NativeScriptData data{.library = library};

  if (const auto ok_or_error = reload(deps, data); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }

  return data;
}

expected<NativeScriptData, NativeScriptError>
NativeScriptCache::generate(dependencies deps, const asset::path& path, const LibraryFlags& flags)
{
  auto library_or_error = deps.get<LibraryCache>().find_or_create(path, deps, path, flags);
  if (!library_or_error.has_value())
  {
    SDE_LOG_ERROR() << "ScriptLibraryInvalid: " << library_or_error.error();
    return make_unexpected(NativeScriptError::kScriptLibraryInvalid);
  }
  return this->generate(deps, library_or_error->handle);
}

}  // namespace sde::game