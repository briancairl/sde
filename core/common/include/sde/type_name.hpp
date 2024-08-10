/**
 * @copyright 2022-present Brian Cairl
 *
 * @file typestr.hpp
 */
#pragma once

// C++ Standard Library
#include <string_view>

namespace sde
{

/**
 * @brief Returns the full-qualified name of a type as a C-style string
 */
template <typename T> std::string_view type_name()
{
  constexpr std::string_view kPrettyFunction{__PRETTY_FUNCTION__};
  constexpr std::string_view kToken{"T = "};
  constexpr auto kNameStartOffset = kPrettyFunction.find(kToken);
  static_assert(kNameStartOffset != std::string_view::npos);
  constexpr auto kNameEndOffset = kPrettyFunction.find(";", kNameStartOffset);
  if constexpr (kNameEndOffset == std::string_view::npos)
  {
    return kPrettyFunction.substr(kNameStartOffset + kToken.size(), kNameEndOffset);
  }
  else
  {
    return kPrettyFunction.substr(kNameStartOffset + kToken.size(), kNameEndOffset - kNameStartOffset - kToken.size());
  }
}

}  // namespace sde
