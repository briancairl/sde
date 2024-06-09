/**
 * @copyright 2024-present Brian Cairl
 *
 * @file geometry_types.hpp
 */
#pragma once

// C++ Standard Library
#include <iosfwd>

// Eigen
#include <Eigen/Dense>

namespace sde
{

template <typename T, std::size_t N> using Vec = Eigen::Matrix<T, N, 1, Eigen::ColMajor>;

template <typename T, std::size_t N, std::size_t M = N> using Mat = Eigen::Matrix<T, N, M, Eigen::ColMajor>;

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

template <typename T, std::size_t Dim> using Bounds = Eigen::AlignedBox<T, Dim>;

using Bounds2i = Bounds<int, 2>;
using Bounds2f = Bounds<float, 2>;

}  // namespace sde

namespace Eigen
{

template <typename T, int Dim> std::ostream& operator<<(std::ostream& os, const AlignedBox<T, Dim>& bounds);

}  // namespace Eigen
