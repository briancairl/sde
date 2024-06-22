/**
 * @copyright 2024-present Brian Cairl
 *
 * @file app.hpp
 */
#pragma once

// C++ Standard Library
#include <bitset>

namespace sde
{

enum class KeyCode : std::size_t
{
  kNum1,
  kNum2,
  kNum3,
  kNum4,
  kNum5,
  kNum6,
  kNum7,
  kNum8,
  kNum9,
  kNum0,
  kQ,
  kW,
  kE,
  kA,
  kS,
  kD,
  kZ,
  kX,
  kC,
  kSpace,
  kLShift,
  kRShift,
  kLCtrl,
  kRCtrl,
  kLAlt,
  kRAlt,
  _Count_
};

static const auto kKeyCount = static_cast<std::size_t>(KeyCode::_Count_);

struct KeyStates
{
  std::bitset<kKeyCount> down{0};
  std::bitset<kKeyCount> pressed{0};
  std::bitset<kKeyCount> released{0};

  constexpr bool isDown(KeyCode code) const { return down[static_cast<std::size_t>(code)]; }
  constexpr bool isPressed(KeyCode code) const { return pressed[static_cast<std::size_t>(code)]; }
  constexpr bool isReleased(KeyCode code) const { return released[static_cast<std::size_t>(code)]; }
};

}  // namespace sde
