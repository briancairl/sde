#include "sde/geometry.hpp"

namespace sde
{

template <typename CornerT> std::ostream& operator<<(std::ostream& os, const Rect<CornerT>& rect)
{
  return os << "{pt0: " << rect.pt0 << ", pt1: " << rect.pt1 << '}';
}

template std::ostream& operator<<(std::ostream& os, const Rect<Vec2i>& rect);
template std::ostream& operator<<(std::ostream& os, const Rect<Vec2f>& rect);
template std::ostream& operator<<(std::ostream& os, const Rect<Vec3f>& rect);

}  // namespace sde

namespace Eigen
{

template <typename T, int Dim> std::ostream& operator<<(std::ostream& os, const AlignedBox<T, Dim>& bounds)
{
  return os << "{min: " << bounds.min().transpose() << ", max: " << bounds.max().transpose() << '}';
}

template <typename T, int Dim> std::ostream& operator<<(std::ostream& os, const Matrix<T, Dim, 1>& vec)
{
  os << "{ ";
  for (int i = 0; i < vec.rows(); ++i)
  {
    os << vec[i] << " ";
  }
  return os << '}';
}

template std::ostream& operator<<(std::ostream& os, const AlignedBox<int, 2>& bounds);
template std::ostream& operator<<(std::ostream& os, const AlignedBox<float, 2>& bounds);
template std::ostream& operator<<(std::ostream& os, const AlignedBox<float, 3>& bounds);

template std::ostream& operator<<(std::ostream& os, const Matrix<int, 2, 1>& bounds);
template std::ostream& operator<<(std::ostream& os, const Matrix<float, 2, 1>& bounds);
template std::ostream& operator<<(std::ostream& os, const Matrix<float, 3, 1>& bounds);
template std::ostream& operator<<(std::ostream& os, const Matrix<float, 4, 1>& bounds);

}  // namespace Eigen
