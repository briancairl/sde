/**
 * @copyright 2024-present Brian Cairl
 *
 * @file geometry_io.hpp
 */
#pragma once

// SDE
#include "sde/geometry.hpp"
#include "sde/serialization.hpp"

namespace sde::serial
{

template <typename Archive, typename T, int N, int M> struct save<Archive, Mat<T, N, M>>
{
  static_assert((N > 0) and (M > 0), "dimensions must be positive values");
  void operator()(Archive& ar, const Mat<T, N, M>& mat) const
  {
    ar << named{"data", make_packet(mat.data(), mat.size())};
  }
};

template <typename Archive, typename T, int N, int M> struct load<Archive, Mat<T, N, M>>
{
  static_assert((N > 0) and (M > 0), "dimensions must be positive values");
  void operator()(Archive& ar, Mat<T, N, M>& mat) const { ar >> named{"data", make_packet(mat.data(), mat.size())}; }
};

template <typename Archive, typename T, int D> struct save<Archive, Bounds<T, D>>
{
  void operator()(Archive& ar, const Bounds<T, D>& bounds) const
  {
    ar << named{"min", bounds.min()};
    ar << named{"max", bounds.max()};
  }
};

template <typename Archive, typename T, int D> struct load<Archive, Bounds<T, D>>
{
  void operator()(Archive& ar, Bounds<T, D>& bounds) const
  {
    Vec<T, D> p_min, p_max;
    ar >> named{"min", p_min};
    ar >> named{"max", p_max};
    bounds = Bounds<T, D>{p_min, p_max};
  }
};

}  // namespace sde::serial
