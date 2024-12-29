// C++ Standard Library
#include <ostream>

// SDE
#include "sde/game/native_script_methods.hpp"
#include "sde/logging.hpp"

namespace sde::game
{

bool NativeScriptMethods::isValid() const
{
  return IterateUntil(*this, [](const auto& fn) { return fn->isValid(); });
}

void NativeScriptMethods::reset()
{
  IterateUntil(*this, [](auto& fn) {
    fn->reset();
    return true;
  });
}

bool NativeScriptMethods::reset(const dl::Library& library)
{
  return IterateUntil(*this, [this, &library](auto& field) {
    auto symbol_or_error = library.get(field.name);
    if (!symbol_or_error.has_value())
    {
      SDE_LOG_ERROR() << field.name << " : " << symbol_or_error.error();
      return false;
    }
    else
    {
      (*field) = std::move(symbol_or_error).value();
    }
    return true;
  });
}

std::ostream& operator<<(std::ostream& os, const NativeScriptMethods& methods)
{
  if (methods.isValid())
  {
    return os << methods.name() << '.' << methods.version();
  }
  return os << "<INVALID>";
}

}  // namespace sde::game