/**
 * @copyright 2024-present Brian Cairl
 *
 * @file geometry.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// Eigen
#include <Eigen/Dense>

// SDE
#include "sde/hash.hpp"

namespace sde
{

template <typename T, int N> using Vec = Eigen::Matrix<T, N, 1, Eigen::ColMajor>;
template <typename T, int N, int M = N> using Mat = Eigen::Matrix<T, N, M, Eigen::ColMajor>;
template <typename T, int Dim> using Bounds = Eigen::AlignedBox<T, Dim>;

using Vec2i = Vec<int, 2>;
using Vec2f = Vec<float, 2>;
using Vec2d = Vec<double, 2>;

using Vec3i = Vec<int, 3>;
using Vec3f = Vec<float, 3>;
using Vec3d = Vec<double, 3>;

using Vec4i = Vec<int, 4>;
using Vec4f = Vec<float, 4>;
using Vec4d = Vec<double, 4>;

using Mat2i = Mat<int, 2>;
using Mat2f = Mat<float, 2>;
using Mat2d = Mat<double, 2>;

using Mat3i = Mat<int, 3>;
using Mat3f = Mat<float, 3>;
using Mat3d = Mat<double, 3>;

using Mat4i = Mat<int, 4>;
using Mat4f = Mat<float, 4>;
using Mat4d = Mat<double, 4>;

using MatXi = Mat<int, Eigen::Dynamic>;
using MatXf = Mat<float, Eigen::Dynamic>;
using MatXd = Mat<double, Eigen::Dynamic>;

using Bounds2i = Bounds<int, 2>;
using Bounds2f = Bounds<float, 2>;

template <typename T, int N, int M> inline bool operator==(const Mat<T, N, M>& lhs, const Mat<T, N, M>& rhs)
{
  return (lhs.array() == rhs.array()).all();
}

template <typename T, int D> inline bool operator==(const Bounds<T, D>& lhs, const Bounds<T, D>& rhs)
{
  return (lhs.min() == rhs.min()) && (lhs.max() == rhs.max());
}

template <typename T, int N, int M> struct Hasher<Eigen::Matrix<T, N, M, Eigen::ColMajor>>
{
  auto operator()(const Eigen::Matrix<T, N, M, Eigen::ColMajor>& m) const
  {
    Hash h{};
    const T* p_beg = m.data();
    const T* p_end = m.data() + m.size();
    for (const T* p = p_beg; p != p_end; ++p)
    {
      h += Hasher<T>{}(*p);
    }
    return h;
  }
};

template <typename T, int Dim> struct Hasher<Eigen::AlignedBox<T, Dim>>
{
  auto operator()(const Eigen::AlignedBox<T, Dim>& box) const { return ComputeHash(box.min(), box.max()); }
};

template <typename CornerT> struct Rect
{
  CornerT pt0;
  CornerT pt1;

  template <typename TranslationT> Rect& operator+=(const TranslationT& translation)
  {
    this->pt0 += translation;
    this->pt1 += translation;
    return *this;
  }

  template <typename TranslationT> Rect& operator-=(const TranslationT& translation)
  {
    this->pt0 -= translation;
    this->pt1 -= translation;
    return *this;
  }

  auto bounds() const
  {
    using Scalar = typename CornerT::Scalar;
    return Bounds<Scalar, CornerT::RowsAtCompileTime>{pt0, pt1};
  }
};

template <typename CornerT> std::ostream& operator<<(std::ostream& os, const Rect<CornerT>& rect);

using Rect2i = Rect<Vec2i>;
using Rect2f = Rect<Vec2f>;

template <typename CornerT> struct Hasher<Rect<CornerT>>
{
  auto operator()(const Rect<CornerT>& rect) const { return ComputeHash(rect.pt0, rect.pt1); }
};

}  // namespace sde

namespace Eigen
{

template <typename T, int Dim> std::ostream& operator<<(std::ostream& os, const AlignedBox<T, Dim>& bounds);

template <typename T, int Dim> std::ostream& operator<<(std::ostream& os, const Matrix<T, Dim, 1>& vec);

}  // namespace Eigen
