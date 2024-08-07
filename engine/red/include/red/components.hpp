#pragma once

// C++ Standard Library
#include <string>

// SDE
#include "sde/geometry.hpp"
#include "sde/resource.hpp"

using namespace sde;

struct Info : Resource<Info>
{
  std::string name;

  auto field_list() { return FieldList(Field{"name", name}); }
};

struct Size : Resource<Size>
{
  Vec2f extent;

  auto field_list() { return FieldList(Field{"extent", extent}); }
};

struct Position : Resource<Position>
{
  Vec2f center;

  auto field_list() { return FieldList(Field{"center", center}); }
};

struct Dynamics : Resource<Dynamics>
{
  Vec2f velocity;
  Vec2f looking;

  auto field_list() { return FieldList(Field{"velocity", velocity}, Field{"looking", looking}); }
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