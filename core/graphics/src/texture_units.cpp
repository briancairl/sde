// C++ Standard Library
#include <ostream>

// SDE
#include "sde/graphics/texture_units.hpp"

namespace sde::graphics
{

std::ostream& operator<<(std::ostream& os, const TextureUnits& tu)
{
  for (std::size_t index = 0; index < tu.slots.size(); ++index)
  {
    os << "[" << index << "] : " << tu.slots[index] << '\n';
  }
  return os;
}

}  // namespace sde::graphics
