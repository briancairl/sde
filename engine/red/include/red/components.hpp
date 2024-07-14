#pragma once

// C++ Standard Library
#include <string>

// SDE
#include "sde/geometry.hpp"

using namespace sde;

struct Info
{
  std::string name;
};

struct Size
{
  Vec2f extent;
};

struct Position
{
  Vec2f center;
};

struct Dynamics
{
  Vec2f velocity;
  Vec2f looking;
};

struct Direction
{
  Vec2f looking;
};

struct Focused
{};

struct Background
{};

struct Midground
{};

struct Foreground
{};

static constexpr std::size_t kGlobalListener = 0;
static constexpr std::size_t kPlayerListener = 1;