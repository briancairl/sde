/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shapes.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// SDE
#include "sde/geometry.hpp"

namespace sde::graphics
{

// clang-format off
inline Vec4f Black(float alpha = 1.0F)   { return Vec4f{0.0F, 0.0F, 0.0F, alpha}; };
inline Vec4f White(float alpha = 1.0F)   { return Vec4f{1.0F, 1.0F, 1.0F, alpha}; };
inline Vec4f Red(float alpha = 1.0F)     { return Vec4f{1.0F, 0.0F, 0.0F, alpha}; };
inline Vec4f Green(float alpha = 1.0F)   { return Vec4f{0.0F, 1.0F, 0.0F, alpha}; };
inline Vec4f Blue(float alpha = 1.0F)    { return Vec4f{0.0F, 0.0F, 1.0F, alpha}; };
inline Vec4f Yellow(float alpha = 1.0F)  { return Vec4f{1.0F, 1.0F, 0.0F, alpha}; };
inline Vec4f Cyan(float alpha = 1.0F)    { return Vec4f{0.0F, 1.0F, 1.0F, alpha}; };
inline Vec4f Magenta(float alpha = 1.0F) { return Vec4f{1.0F, 0.0F, 1.0F, alpha}; };
// clang-format on

}  // namespace sde::graphics
