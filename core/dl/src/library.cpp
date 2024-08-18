// C++ Standard Library
#include <ostream>

/// LibDL
#include <dlfcn.h>

// SDE
#include "sde/dl/library.hpp"
#include "sde/logging.hpp"

namespace sde::dl
{

Library::~Library() { this->reset(); }

void Library::reset()
{
  if (handle_ == nullptr)
  {
    return;
  }
  dlclose(handle_);
  handle_ = nullptr;
}

void Library::swap(Library& other) { std::swap(other.handle_, this->handle_); }

Library::Library(Library&& other) { this->swap(other); }

Library& Library::operator=(Library&& other)
{
  this->swap(other);
  return *this;
}

expected<Symbol, Error> Library::get(const char* symbol) const
{
  if (handle_ == nullptr)
  {
    return make_unexpected(Error{"invalid"});
  }
  const auto sym_ptr = dlsym(handle_, symbol);
  if (sym_ptr == nullptr)
  {
    return make_unexpected(Error{dlerror()});
  }
  return Symbol{sym_ptr};
}

expected<Library, Error> Library::load(const char* library_path)
{
  const auto lib_ptr = dlopen(library_path, RTLD_LAZY | RTLD_GLOBAL);
  if (lib_ptr == nullptr)
  {
    return make_unexpected(Error{dlerror()});
  }
  return Library{lib_ptr};
}

}  // namespace sde::dl
