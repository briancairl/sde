/**
 * @copyright 2024-present Brian Cairl
 *
 * @file native_script_typedefs.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>

using script_id_t = std::size_t;
using script_version_t = std::size_t;
using ScriptInstanceAllocator = void*(std::size_t);
using ScriptInstanceDeallocator = void(void*);
