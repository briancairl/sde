/**
 * @copyright 2024-present Brian Cairl
 *
 * @file geometry.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>

// SDE
#include "sde/geometry_types.hpp"

namespace sde
{

template <typename T> using remove_cv_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <typename PointT, typename... OtherPointTs> auto toBounds(const PointT& first, OtherPointTs&&... others)
{
  using ScalarT = typename PointT::Scalar;
  static constexpr int Rows = PointT::RowsAtCompileTime;
  static constexpr int Cols = PointT::ColsAtCompileTime;
  static_assert(Rows > 0, "point type must not have dynamic row count");
  static_assert(Cols == 1, "point must be a column vector");
  if constexpr (sizeof...(others) > 0)
  {
    static_assert((std::is_same_v<PointT, remove_cv_ref_t<OtherPointTs>> and ...), "point types do not match");
    Bounds<ScalarT, Rows> aabb;
    aabb.extend(first);
    [[maybe_unused]] auto dummy = ((aabb.extend(std::forward<OtherPointTs>(others)), 1) + ...);
    return aabb;
  }
  else
  {
    return Bounds<ScalarT, Rows>{PointT::Zero(), first};
  }
}

template <typename T, int Dim> decltype(auto) toExtents(const Bounds<T, Dim>& bounds)
{
  return bounds.max() - bounds.min();
}

template <typename T, int Dim> bool isEmpty(const Bounds<T, Dim>& bounds) { return bounds.volume() == 0; }

template <typename T, int Dim> Bounds<T, Dim> operator&(const Bounds<T, Dim>& lhs, const Bounds<T, Dim>& rhs)
{
  return lhs.intersection(rhs);
}

template <typename T, int MatDim, int VecDim>
Vec<T, VecDim> transform(const Mat<T, MatDim>& affine, const Vec<T, VecDim>& v)
{
  static_assert(MatDim == VecDim + 1);
  return affine.template block<VecDim, VecDim>(0, 0) * v +
    affine.template block<VecDim, 1>(0, VecDim) / affine(VecDim, VecDim);
}

template <typename T, int MatDim, int VecDim>
Bounds<T, VecDim> transform(const Mat<T, MatDim>& affine, const Bounds<T, VecDim>& v)
{
  return toBounds(transform(affine, v.min()), transform(affine, v.max()));
}


}  // namespace sde
