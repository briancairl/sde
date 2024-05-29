/**
 * @copyright 2024-present Brian Cairl
 *
 * @file shader.hpp
 */
#pragma once

// C++ Standard Library
#include <cstdint>
#include <iosfwd>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// SDE
#include "sde/asset.hpp"
#include "sde/expected.hpp"
#include "sde/graphics/shader_handle.hpp"
#include "sde/graphics/typedef.hpp"

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
  kLoadFailure,
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

std::ostream& operator<<(std::ostream& os, ShaderComponents components);

struct ShaderVariables
{
  std::vector<ShaderVariable> layout;
  std::vector<ShaderVariable> uniforms;
};

std::ostream& operator<<(std::ostream& os, const ShaderVariables& variables);

/**
 * @brief Information about an active shader
 */
struct ShaderInfo
{
  ShaderComponents components;
  ShaderVariables variables;
  native_shader_id_t native_id;
};

std::ostream& operator<<(std::ostream& os, const ShaderInfo& error);

[[nodiscard]] bool hasLayout(const ShaderInfo& info, std::string_view key, ShaderVariableType type, std::size_t index);

[[nodiscard]] bool hasUniform(const ShaderInfo& info, std::string_view key, ShaderVariableType type);

class ShaderCache
{
public:
  ShaderCache() = default;

  ~ShaderCache();

  bool remove(const ShaderHandle& index);

  [[nodiscard]] expected<ShaderHandle, ShaderError> toShader(std::string_view source);

  [[nodiscard]] expected<ShaderHandle, ShaderError> load(const asset::path& shader_path);

  const ShaderInfo* get(ShaderHandle shader) const;

private:
  using ShaderCacheMap = std::unordered_map<ShaderHandle, ShaderInfo, ResourceHandleHash>;

  ShaderHandle last_shader_handle_ = ShaderHandle::null();

  ShaderCacheMap shaders_;

  ShaderHandle getNextShaderHandle() { return ShaderHandle{last_shader_handle_.id() + 1UL}; }
};

}  // namespace sde::graphics
