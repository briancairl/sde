// C++ Standard Library
#include <ostream>

// SDE
#include "sde/serialization.hpp"
#include "sde/serialization_binary_file.hpp"
#include "sde/serialization_fwd.hpp"

namespace sde::serial
{

template class binary_oarchive<file_handle_ostream>;
template class binary_iarchive<file_handle_istream>;

}  // namespace sde::serial