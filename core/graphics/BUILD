cc_library(
  name="graphics_impl__opengl",
  hdrs=["src/opengl.inl"],
  strip_include_prefix="src",
  deps=["//platform/glad:glad", "@glfw//:glfw"],
  visibility=["//visibility:private"]
)



cc_library(
  name="graphics_impl__common",
  hdrs=[
    "include/sde/graphics/typecode.hpp",
    "include/sde/graphics/typedef.hpp",
  ],
  srcs=[
    "src/typecode.cpp"
  ],
  strip_include_prefix="include",
  deps=[],
  visibility=["//visibility:private"]
)



cc_library(
  name="graphics_impl__image__stb",
  hdrs=[
    "include/sde/graphics/image.hpp",
    "include/sde/graphics/image_fwd.hpp",
  ],
  srcs=[
    "src/image.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    ":graphics_impl__common",
    "//core/common:core",
    "//core/common:expected",
    "//core/common:geometry",
    "//core/common:asset",
    "@stb//:stb",
  ],
  visibility=["//visibility:private"]
)




cc_library(
  name="graphics_impl__renderer__hdrs",
  hdrs=[
    "include/sde/graphics/assets.hpp",
    "include/sde/graphics/assets_fwd.hpp",
    "include/sde/graphics/colors.hpp",
    "include/sde/graphics/debug.hpp",
    "include/sde/graphics/render_buffer.hpp",
    "include/sde/graphics/render_buffer_fwd.hpp",
    "include/sde/graphics/render_target_fwd.hpp",
    "include/sde/graphics/render_target_handle.hpp",
    "include/sde/graphics/render_target.hpp",
    "include/sde/graphics/renderer.hpp",
    "include/sde/graphics/renderer_fwd.hpp",
    "include/sde/graphics/sprite.hpp",
    "include/sde/graphics/sprite_io.hpp",
    "include/sde/graphics/shader.hpp",
    "include/sde/graphics/shader_fwd.hpp",
    "include/sde/graphics/shader_io.hpp",
    "include/sde/graphics/shader_handle.hpp",
    "include/sde/graphics/shapes.hpp",
    "include/sde/graphics/shapes_fwd.hpp",
    "include/sde/graphics/font.hpp",
    "include/sde/graphics/font_fwd.hpp",
    "include/sde/graphics/font_handle.hpp",
    "include/sde/graphics/type_set.hpp",
    "include/sde/graphics/type_set_fwd.hpp",
    "include/sde/graphics/type_set_handle.hpp",
    "include/sde/graphics/type_setter.hpp",
    "include/sde/graphics/texture.hpp",
    "include/sde/graphics/texture_io.hpp",
    "include/sde/graphics/texture_fwd.hpp",
    "include/sde/graphics/texture_handle.hpp",
    "include/sde/graphics/texture_units.hpp",
    "include/sde/graphics/tile_set.hpp",
    "include/sde/graphics/tile_set_fwd.hpp",
    "include/sde/graphics/tile_set_io.hpp",
    "include/sde/graphics/tile_set_handle.hpp",
    "include/sde/graphics/tile_map.hpp",
    "include/sde/graphics/tile_map_io.hpp",
    "include/sde/graphics/window.hpp",
    "include/sde/graphics/window_fwd.hpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/common:core",
    "//core/common:expected",
    "//core/common:geometry",
    "//core/common:logging",
    "//core/common:resource",
    ":graphics_impl__common",
    ":graphics_impl__image__stb"
  ],
  visibility=["//visibility:private"]
)



cc_library(
  name="graphics_impl__renderer__opengl_internal",
  hdrs=[],
  strip_include_prefix="include",
  deps=[":graphics_impl__renderer__hdrs", "//platform/glad:glad", "@glfw//:glfw",],
  visibility=["//visibility:private"]
)




config_setting(
    name = "debug",
    values = {"compilation_mode": "dbg"},
)



graphics_impl__renderer__opengl_debug_selector = select({
    ":debug": ["src/debug.cpp"],
    "//conditions:default": ["src/debug_null.cpp"],
})

cc_library(
  name="graphics_impl__renderer__opengl",
  srcs=[
    "src/font.cpp",
    "src/renderer.cpp",
    "src/render_target.cpp",
    "src/shader.cpp",
    "src/shader_io.cpp",
    "src/shapes.cpp",
    "src/sprite.cpp",
    "src/type_set.cpp",
    "src/type_setter.cpp",
    "src/texture.cpp",
    "src/texture_io.cpp",
    "src/texture_units.cpp",
    "src/tile_set.cpp",
    "src/tile_map.cpp",
  ] + graphics_impl__renderer__opengl_debug_selector,
  deps=[
    "//core/serialization",
    "//core/serialization/std",
    ":graphics_impl__renderer__opengl_internal",
    ":graphics_impl__renderer__hdrs",
    ":graphics_impl__common",
    ":graphics_impl__opengl",
    "@freetype2//:freetype2"
  ],
  visibility=["//visibility:private"]
)


alias(
  name="graphics",
  actual=":graphics_impl__renderer__opengl",
  visibility=["//visibility:public"]
)


platform_glfw_debug_selector = select({
    ":debug": ["SDE_GLFW_DEBUG=1"],
    "//conditions:default": ["SDE_GLFW_DEBUG=0"]
})

cc_library(
  name="window",
  hdrs=["include/sde/graphics/window.hpp"],
  srcs=["src/window.cpp"],
  defines=platform_glfw_debug_selector,
  strip_include_prefix="include",
  deps=[
    ":graphics_impl__renderer__opengl",
    "//core/common:logging",
    "//core/common:geometry",
    "//platform/glad:glad",
    "@glfw//:glfw",
  ],
  visibility=["//visibility:public"]
)
