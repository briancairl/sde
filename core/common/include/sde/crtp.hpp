/**
 * @copyright 2024-present Brian Cairl
 *
 * @file crtp.hpp
 */
#pragma once

// C++ Standard Library
#include <type_traits>

namespace sde
{

template <typename DerivedT> class crtp_base;

template <template <typename> class CRTPBaseTemplate, typename DerivedT> class crtp_base<CRTPBaseTemplate<DerivedT>>
{
protected:
  constexpr DerivedT* derived_ptr()
  {
    if constexpr (std::is_polymorphic_v<DerivedT>)
    {
      return static_cast<DerivedT*>(this);
    }
    else
    {
      return reinterpret_cast<DerivedT*>(this);
    }
  }
  constexpr const DerivedT* derived_ptr() const
  {
    if constexpr (std::is_polymorphic_v<DerivedT>)
    {
      return static_cast<const DerivedT*>(this);
    }
    else
    {
      return reinterpret_cast<const DerivedT*>(this);
    }
  }
  constexpr DerivedT& derived() { return *derived_ptr(); }
  constexpr const DerivedT& derived() const { return *derived_ptr(); }
};

}  // namespace sde