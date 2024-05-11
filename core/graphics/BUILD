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
  srcs=[],
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
    "//core/common:expected",
    "//core/common:resources",
    "@stb//:stb",
  ],
  visibility=["//visibility:private"]
)




cc_library(
  name="graphics_impl__renderer__hdrs",
  hdrs=[
    "include/sde/graphics/renderer.hpp",
    "include/sde/graphics/texture.hpp"
  ],
  strip_include_prefix="include",
  deps=[
    "//core/common:core",
    "//core/common:expected",
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
    ":debug": [], #["src/debug.cpp"],
    "//conditions:default": [] #["src/debug_null.cpp"],
})

cc_library(
  name="graphics_impl__renderer__opengl",
  srcs=[
    "src/renderer.cpp",
    "src/texture.cpp",
  ] + graphics_impl__renderer__opengl_debug_selector,
  deps=[
    ":graphics_impl__renderer__opengl_internal",
    ":graphics_impl__renderer__hdrs",
    ":graphics_impl__common",
    ":graphics_impl__opengl",],
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
  name="platform",
  hdrs=["include/sde/graphics/platform.hpp"],
  srcs=["src/platform.cpp"],
  defines=platform_glfw_debug_selector,
  strip_include_prefix="include",
  deps=[
    "//core/common:logging",
    "//platform/glad:glad",
    "@glfw//:glfw"
  ],
  visibility=["//visibility:public"]
)
