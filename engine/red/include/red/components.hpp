#pragma once

// SDE
#include "sde/game/scene_fwd.hpp"
#include "sde/geometry.hpp"
#include "sde/resource.hpp"
#include "sde/string.hpp"

using namespace sde;

struct Info : Resource<Info>
{
  sde::string name;

  auto field_list() { return FieldList(Field{"name", name}); }
};

struct Size : Resource<Size>
{
  Vec2f extent;

  auto field_list() { return FieldList(Field{"extent", extent}); }
};

struct DebugWireFrame : Resource<DebugWireFrame>
{
  Vec4f color;
  auto field_list() { return FieldList(Field{"color", color}); }
};

struct Position : Resource<Position>
{
  Vec2f center;

  auto field_list() { return FieldList(Field{"center", center}); }
};

struct TransformQuery : Resource<TransformQuery>
{
  Mat3f world_from_viewport = Mat3f::Identity();
  auto field_list() { return FieldList(Field{"world_from_viewport", world_from_viewport}); }
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
