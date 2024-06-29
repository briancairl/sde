#pragma once

// C++ Standard Library
#include <string>

// SDE
#include "sde/geometry_types.hpp"

using namespace sde;

struct Info
{
  std::string name;
};

struct Size
{
  Vec2f extent;
};

struct State
{
  Vec2f position;
  Vec2f velocity;
  Vec2f looking;
};

struct Direction
{
  Vec2f looking;
};
