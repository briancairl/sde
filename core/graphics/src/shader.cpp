// C++ Standard Library
#include <algorithm>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <ostream>
#include <sstream>

// Backend
#include "opengl.inl"

// SDE
#include "sde/graphics/image.hpp"
#include "sde/graphics/shader.hpp"
#include "sde/logging.hpp"

namespace sde::graphics
{
namespace
{

struct ShaderSourceParts
{
  std::string_view vert;
  std::string_view frag;
  std::string_view geom;
};

ShaderSourceParts toShaderSourceParts(std::string_view source)
{
  static constexpr std::string_view kPartDelimeter{"---"};

  const auto beg_of_vert_pos = 0UL;
  const auto end_of_vert_pos = source.find(kPartDelimeter, beg_of_vert_pos);

  if (end_of_vert_pos == std::string_view::npos)
  {
    // clang-format off
    return {
      .vert = source.substr(beg_of_vert_pos, end_of_vert_pos),
      .frag = {},
      .geom = {}
    };
    // clang-format on
  }

  const auto beg_of_frag_pos = end_of_vert_pos + kPartDelimeter.size();
  const auto end_of_frag_pos = source.find(kPartDelimeter, beg_of_frag_pos);

  if (end_of_frag_pos == std::string_view::npos)
  {
    // clang-format off
    return {
      .vert = source.substr(beg_of_vert_pos, end_of_vert_pos - beg_of_vert_pos),
      .frag = source.substr(beg_of_frag_pos, end_of_frag_pos),
      .geom = {}
    };
    // clang-format on
  }

  const auto beg_of_geom_pos = end_of_frag_pos + kPartDelimeter.size();

  // clang-format off
  return {
    .vert = source.substr(beg_of_vert_pos, end_of_vert_pos - beg_of_vert_pos),
    .frag = source.substr(beg_of_frag_pos, end_of_frag_pos - end_of_frag_pos),
    .geom = source.substr(beg_of_geom_pos, std::string_view::npos)
  };
  // clang-format on
}

std::pair<std::string_view, std::string_view> splitTypeAndExtent(std::string_view type_str)
{
  const auto beg = type_str.find('[');
  if (beg == std::string_view::npos)
  {
    return {type_str, {}};
  }
  const auto end = type_str.find(']');
  return {type_str.substr(0, beg), type_str.substr(beg + 1, end - beg - 1)};
}

int toInteger(std::string_view intstr)
{
  if (intstr.empty())
  {
    return 1;
  }
  int v = 0;
  SDE_ASSERT_EQ(std::from_chars(intstr.data(), intstr.data() + intstr.size(), v).ec, std::errc());
  return v;
}

ShaderVariableType toShaderVariableType(std::string_view type_str)
{
  if (type_str == "int")
  {
    return ShaderVariableType::kInt;
  }

  if (type_str == "float")
  {
    return ShaderVariableType::kFloat;
  }

  if (type_str == "vec2")
  {
    return ShaderVariableType::kVec2;
  }

  if (type_str == "vec3")
  {
    return ShaderVariableType::kVec3;
  }

  if (type_str == "vec4")
  {
    return ShaderVariableType::kVec4;
  }

  if (type_str == "mat2")
  {
    return ShaderVariableType::kMat2;
  }

  if (type_str == "mat3")
  {
    return ShaderVariableType::kMat3;
  }

  if (type_str == "mat4")
  {
    return ShaderVariableType::kMat4;
  }

  if (type_str == "sampler2D")
  {
    return ShaderVariableType::kSampler2;
  }

  if (type_str == "sampler3D")
  {
    return ShaderVariableType::kSampler3;
  }

  SDE_SHOULD_NEVER_HAPPEN("Unhandled type_str");
}

std::size_t
parseLayoutVariables(sde::vector<ShaderVariable>& variables, std::string_view source, std::size_t next_start_pos = 0)
{
  static constexpr std::string_view kLayoutToken = "layout ";
  static constexpr std::string_view kInputToken = "in ";
  static constexpr std::string_view kVarNameBegToken = " ";
  static constexpr std::string_view kVarNameEndToken = ";";
  if (source.empty())
  {
    return next_start_pos;
  }
  while (true)
  {
    const auto layout_beg_pos = source.find(kLayoutToken, next_start_pos);
    if (layout_beg_pos == std::string_view::npos)
    {
      break;
    }

    auto var_type_beg_pos = source.find(kInputToken, layout_beg_pos);
    if (layout_beg_pos != std::string_view::npos)
    {
      var_type_beg_pos += kInputToken.size();
    }
    else
    {
      break;
    }

    const auto var_type_end_pos = source.find(kVarNameBegToken, var_type_beg_pos);
    if (var_type_end_pos == std::string_view::npos)
    {
      break;
    }

    const auto var_name_beg_pos = var_type_end_pos + kVarNameBegToken.size();
    const auto var_name_end_pos = source.find(kVarNameEndToken, var_name_beg_pos);
    if (var_name_end_pos == std::string_view::npos)
    {
      break;
    }

    const auto key = source.substr(var_name_beg_pos, var_name_end_pos - var_name_beg_pos);
    const auto [type_part, extent_part] =
      splitTypeAndExtent(source.substr(var_type_beg_pos, var_type_end_pos - var_type_beg_pos));
    variables.push_back(
      {.key = std::string{key},
       .type = toShaderVariableType(type_part),
       .size = static_cast<std::size_t>(toInteger(extent_part))});

    next_start_pos = var_name_end_pos;
  }
  return next_start_pos;
}

std::size_t
parseUniformVariables(sde::vector<ShaderVariable>& variables, std::string_view source, std::size_t next_start_pos = 0)
{
  static constexpr std::string_view kUniformToken = "uniform ";
  static constexpr std::string_view kVarNameBegToken = " ";
  static constexpr std::string_view kVarNameEndToken = ";";
  if (source.empty())
  {
    return next_start_pos;
  }
  while (true)
  {
    const auto uniform_beg_pos = source.find(kUniformToken, next_start_pos);
    if (uniform_beg_pos == std::string_view::npos)
    {
      break;
    }

    const auto var_type_beg_pos = uniform_beg_pos + kUniformToken.size();
    const auto var_type_end_pos = source.find(kVarNameBegToken, var_type_beg_pos);
    if (var_type_end_pos == std::string_view::npos)
    {
      break;
    }

    const auto var_name_beg_pos = var_type_end_pos + kVarNameBegToken.size();
    const auto var_name_end_pos = source.find(kVarNameEndToken, var_name_beg_pos);
    if (var_name_end_pos == std::string_view::npos)
    {
      break;
    }

    const auto key = source.substr(var_name_beg_pos, var_name_end_pos - var_name_beg_pos);
    const auto [type_part, extent_part] =
      splitTypeAndExtent(source.substr(var_type_beg_pos, var_type_end_pos - var_type_beg_pos));
    variables.push_back(
      {.key = std::string{key},
       .type = toShaderVariableType(type_part),
       .size = static_cast<std::size_t>(toInteger(extent_part))});

    next_start_pos = var_name_end_pos;
  }
  return next_start_pos;
}

void addShaderVersionInfo(std::ostream& os)
{
  GLint major, minor;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);
  os << "#version " << major << minor << 0;
}

template <GLenum ShaderType> native_shader_id_t createShaderFromSource(std::string_view source)
{
  native_shader_id_t shader_id = glCreateShader(ShaderType);

  const auto source_with_version = [&source]() -> std::string {
    std::ostringstream oss;
    addShaderVersionInfo(oss);
    oss << '\n';
    oss << source;
    oss << '\n';
    return oss.str();
  }();

  // Transfer source code
  const char* c_code = source_with_version.data();
  const GLint c_len = source_with_version.size();
  glShaderSource(shader_id, 1, &c_code, &c_len);

  // Compile component shader code and check for errors
  glCompileShader(shader_id);

  // Check compilation status
  GLint success;
  glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

  if (success == GL_TRUE)
  {
    return shader_id;
  }

  // Get shader compilation error
  GLint len;
  glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &len);

  std::basic_string<GLchar> info_log_contents(static_cast<std::size_t>(len), '*');
  GLsizei written;
  glGetShaderInfoLog(shader_id, len, &written, info_log_contents.data());

  SDE_LOG_ERROR_FMT("shader compilation error: %s in:\n%s", info_log_contents.data(), source_with_version.data());

  glDeleteShader(shader_id);

  return 0;
}

native_shader_id_t createShaderProgram(native_shader_id_t vert, native_shader_id_t frag, native_shader_id_t geom)
{
  native_shader_id_t program_id = glCreateProgram();

  // Attach shader parts to program
  glAttachShader(program_id, vert);
  glAttachShader(program_id, frag);
  if (geom != 0)
  {
    glAttachShader(program_id, geom);
  }

  // Link shader program components
  glLinkProgram(program_id);

  // Detach shaders
  glDetachShader(program_id, vert);
  glDetachShader(program_id, frag);
  if (geom != 0)
  {
    glDetachShader(program_id, geom);
  }

  // Check compilation status
  GLint success;
  glGetProgramiv(program_id, GL_LINK_STATUS, &success);

  if (success == GL_TRUE)
  {
    return program_id;
  }

  // Get shader compilation error
  GLint len;
  glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len);

  std::basic_string<GLchar> info_log_contents(static_cast<std::size_t>(len), '*');
  GLsizei written;
  glGetProgramInfoLog(program_id, len, &written, info_log_contents.data());

  SDE_LOG_ERROR_FMT("shader linkage error: %s", info_log_contents.data());

  glDeleteProgram(program_id);

  return 0;
}

expected<void, ShaderError> compile(Shader& shader, std::string_view source)
{
  const auto source_parts = toShaderSourceParts(source);

  ShaderComponents components{
    .has_vert = !source_parts.vert.empty(),
    .has_frag = !source_parts.frag.empty(),
    .has_geom = !source_parts.geom.empty(),
  };

  SDE_ASSERT(components.has_vert, "shader source missing vertex part");
  SDE_ASSERT(components.has_frag, "shader source missing fragment part");

  ShaderVariables variables;

  native_shader_id_t vert_shader_id = 0;
  if (components.has_vert)
  {
    std::size_t next_start_pos = 0;
    next_start_pos = parseLayoutVariables(variables.layout, source_parts.vert, next_start_pos);
    next_start_pos = parseUniformVariables(variables.uniforms, source_parts.vert, next_start_pos);
    vert_shader_id = createShaderFromSource<GL_VERTEX_SHADER>(source_parts.vert);
    if (vert_shader_id == 0)
    {
      SDE_LOG_DEBUG("VertShaderCompilationFailure");
      return make_unexpected(ShaderError::kVertShaderCompilationFailure);
    }
  }

  native_shader_id_t frag_shader_id = 0;
  if (components.has_frag)
  {
    parseUniformVariables(variables.uniforms, source_parts.frag);
    frag_shader_id = createShaderFromSource<GL_FRAGMENT_SHADER>(source_parts.frag);
    if (frag_shader_id == 0)
    {
      SDE_LOG_DEBUG("FragShaderCompilationFailure");
      return make_unexpected(ShaderError::kFragShaderCompilationFailure);
    }
  }

  native_shader_id_t geom_shader_id = 0;
  if (components.has_geom)
  {
    parseUniformVariables(variables.uniforms, source_parts.geom);
    geom_shader_id = createShaderFromSource<GL_GEOMETRY_SHADER>(source_parts.geom);
    if (geom_shader_id == 0)
    {
      SDE_LOG_DEBUG("GeomShaderCompilationFailure");
      return make_unexpected(ShaderError::kGeomShaderCompilationFailure);
    }
  }

  shader.components = components;
  shader.variables = std::move(variables);
  shader.native_id = NativeShaderID{createShaderProgram(vert_shader_id, frag_shader_id, geom_shader_id)};
  return {};
}

}  // namespace

template <typename T> std::ostream& operator<<(std::ostream& os, const sde::vector<T>& vec)
{
  os << '[';
  for (const auto& el : vec)
  {
    os << ' ' << el << ',';
  }
  os << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, ShaderVariableType value_type)
{
  switch (value_type)
  {
  case ShaderVariableType::kInt:
    return os << "Int";
  case ShaderVariableType::kFloat:
    return os << "Float";
  case ShaderVariableType::kVec2:
    return os << "Vec2";
  case ShaderVariableType::kVec3:
    return os << "Vec3";
  case ShaderVariableType::kVec4:
    return os << "Vec4";
  case ShaderVariableType::kMat2:
    return os << "Mat2";
  case ShaderVariableType::kMat3:
    return os << "Mat3";
  case ShaderVariableType::kMat4:
    return os << "Mat4";
  case ShaderVariableType::kSampler2:
    return os << "Sampler2";
  case ShaderVariableType::kSampler3:
    return os << "Sampler3";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const ShaderVariables& variables)
{
  // clang-format off
  os << "{ layout: "     << variables.layout
     << ", uniforms: "   << variables.uniforms
     << " }";
  // clang-format on
  return os;
}

std::ostream& operator<<(std::ostream& os, const ShaderVariable& value)
{
  return os << "{ key: " << value.key << ", type: " << value.type << " }";
}

std::ostream& operator<<(std::ostream& os, ShaderError error)
{
  switch (error)
  {
  case ShaderError::kElementAlreadyExists:
    return os << "ElementAlreadyExists";
  case ShaderError::kInvalidHandle:
    return os << "InvalidHandle";
  case ShaderError::kAssetNotFound:
    return os << "AssetNotFound";
  case ShaderError::kLinkageFailure:
    return os << "LinkageFailure";
  case ShaderError::kVertShaderCompilationFailure:
    return os << "VertShaderCompilationFailure";
  case ShaderError::kFragShaderCompilationFailure:
    return os << "FragShaderCompilationFailure";
  case ShaderError::kGeomShaderCompilationFailure:
    return os << "GeomShaderCompilationFailure";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, ShaderComponents components)
{
  os << '[';
  if (static_cast<bool>(components.has_vert))
  {
    os << " Vertex";
  }
  if (static_cast<bool>(components.has_frag))
  {
    os << " Fragment";
  }
  if (static_cast<bool>(components.has_geom))
  {
    os << " Geometry";
  }
  os << " ]";
  return os;
}

std::ostream& operator<<(std::ostream& os, const Shader& info)
{
  // clang-format off
  os << "{ components: " << info.components
     << ", variables: "  << info.variables
     << " }";
  // clang-format on
  return os;
}

bool hasLayout(const Shader& info, std::string_view key, ShaderVariableType type, std::size_t index)
{
  return (index < info.variables.layout.size()) && (key == info.variables.layout[index].key) &&
    (type == info.variables.layout[index].type);
}

bool hasUniform(const Shader& info, std::string_view key, ShaderVariableType type)
{
  return std::find_if(
           std::begin(info.variables.uniforms),
           std::end(info.variables.uniforms),
           [key, type](const auto& var) -> bool { return (var.key == key) && (var.type == type); }) !=
    std::end(info.variables.uniforms);
}

void NativeShaderDeleter::operator()(native_shader_id_t id) const
{
  SDE_LOG_DEBUG_FMT("glDeleteTextures(1, &%u)", id);
  glDeleteTextures(1, &id);
}

expected<void, ShaderError> ShaderCache::reload(Shader& shader)
{
  // Check if image point is valid
  if (!asset::exists(shader.path))
  {
    SDE_LOG_DEBUG("AssetNotFound");
    return make_unexpected(ShaderError::kAssetNotFound);
  }

  std::ifstream ifs{shader.path};
  SDE_LOG_INFO_FMT("shader loaded from disk: %s", shader.path.string().c_str());
  std::stringstream shader_source_code;
  shader_source_code << ifs.rdbuf();
  return compile(shader, shader_source_code.str());
}

expected<void, ShaderError> ShaderCache::unload(Shader& shader)
{
  shader.native_id = NativeShaderID{0};
  return {};
}

expected<Shader, ShaderError> ShaderCache::generate(const asset::path& path)
{
  SDE_LOG_INFO_FMT("loading: %s", path.string().c_str());
  Shader shader{.path = path, .components = {}, .variables = {}, .native_id = NativeShaderID{0}};
  if (auto ok_or_error = reload(shader); !ok_or_error.has_value())
  {
    return make_unexpected(ok_or_error.error());
  }
  return shader;
}

}  // namespace sde::graphics
