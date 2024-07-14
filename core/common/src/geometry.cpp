#include "sde/geometry.hpp"

namespace Eigen
{

template <typename T, int Dim> std::ostream& operator<<(std::ostream& os, const AlignedBox<T, Dim>& bounds)
{
  return os << "{min: " << bounds.min().transpose() << ", max: " << bounds.max().transpose() << '}';
}

template std::ostream& operator<<(std::ostream& os, const AlignedBox<int, 2>& bounds);
template std::ostream& operator<<(std::ostream& os, const AlignedBox<float, 2>& bounds);
template std::ostream& operator<<(std::ostream& os, const AlignedBox<float, 3>& bounds);

}  // namespace Eigen
