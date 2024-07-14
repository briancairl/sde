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
public:
  using fundemental_type = CRTPBaseTemplate<DerivedT>;
  fundemental_type& fundemental() { return static_cast<fundemental_type&>(*this); }
  const fundemental_type& fundemental() const { return static_cast<const fundemental_type&>(*this); }

protected:
  constexpr DerivedT* derived_ptr() { return static_cast<DerivedT*>(this); }
  constexpr const DerivedT* derived_ptr() const { return static_cast<const DerivedT*>(this); }
  constexpr DerivedT& derived() { return *derived_ptr(); }
  constexpr const DerivedT& derived() const { return *derived_ptr(); }
};

namespace detail
{

template <typename T, typename Eval = decltype(std::declval<const T&>().fundemental())>
constexpr bool call_fundemental([[maybe_unused]] const T* _)
{
  return true;
}

constexpr bool call_fundemental(...) { return false; }

}  // namespace detail

template <typename T>
struct has_fundemental : std::integral_constant<bool, detail::call_fundemental(std::add_pointer_t<T>{nullptr})>
{};

template <typename T> constexpr bool has_fundemental_v = has_fundemental<T>::value;

}  // namespace sde