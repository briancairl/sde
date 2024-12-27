/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shader.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <string_view>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/graphics/shader_fwd.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/typedef.hpp"
#include "sde/resource.hpp"
#include "sde/resource_cache.hpp"
#include "sde/unique_resource.hpp"
#include "sde/vector.hpp"

namespace sde::graphics
{

/**
 * @brief Possible shader variable types
 */
enum class ShaderVariableType
{
  kInt,
  kFloat,
  kVec2,
  kVec3,
  kVec4,
  kMat2,
  kMat3,
  kMat4,
  kSampler2,
  kSampler3,
};

std::ostream& operator<<(std::ostream& os, ShaderVariableType value_type);

/**
 * @brief A mutable shader value
 */
struct ShaderVariable
{
  std::string key;
  ShaderVariableType type;
  std::size_t size = 1;
};

std::ostream& operator<<(std::ostream& os, const ShaderVariable& value);

/**
 * @brief Possible ShaderSource creation errors
 */
enum class ShaderError
{
  kElementAlreadyExists,
  kInvalidHandle,
  kAssetNotFound,
  kLinkageFailure,
  kVertShaderCompilationFailure,
  kFragShaderCompilationFailure,
  kGeomShaderCompilationFailure,
};

std::ostream& operator<<(std::ostream& os, ShaderError error);

struct ShaderComponents
{
  std::uint8_t has_vert : 1;
  std::uint8_t has_frag : 1;
  std::uint8_t has_geom : 1;
  std::uint8_t __pad__ : 5;
};

[[nodiscard]] constexpr bool isValid(ShaderComponents components)
{
  return components.has_vert and components.has_frag;
}

std::ostream& operator<<(std::ostream& os, ShaderComponents components);

struct ShaderVariables
{
  sde::vector<ShaderVariable> layout;
  sde::vector<ShaderVariable> uniforms;
};

std::ostream& operator<<(std::ostream& os, const ShaderVariables& variables);


struct NativeShaderDeleter
{
  void operator()(native_shader_id_t id) const;
};

using NativeShaderID = UniqueResource<native_shader_id_t, NativeShaderDeleter>;

/**
 * @brief Information about an active shader
 */
struct Shader : Resource<Shader>
{
  asset::path path = {};
  ShaderComponents components = {};
  ShaderVariables variables = {};
  NativeShaderID native_id = NativeShaderID{0};

  auto field_list()
  {
    return FieldList(
      (Field{"path", path}),
      (_Stub{"components", components}),
      (_Stub{"variables", variables}),
      (_Stub{"native_id", native_id}));
  }

  [[nodiscard]] constexpr bool isValid() const { return ::sde::graphics::isValid(components); }
};

[[nodiscard]] bool hasLayout(const Shader& info, std::string_view key, ShaderVariableType type, std::size_t index);

[[nodiscard]] bool hasUniform(const Shader& info, std::string_view key, ShaderVariableType type);

class ShaderCache : public ResourceCache<ShaderCache>
{
  friend fundemental_type;

private:
  static expected<void, ShaderError> reload(dependencies deps, Shader& shader);
  static expected<void, ShaderError> unload(dependencies deps, Shader& shader);
  expected<Shader, ShaderError> generate(dependencies deps, const asset::path& path);
};

}  // namespace sde::graphics
