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
    "//core/common:expected",
    "//core/common:resources",
    "@stb//:stb",
  ],
  visibility=["//visibility:private"]
)




cc_library(
  name="graphics_impl__renderer__hdrs",
  hdrs=[
    "include/sde/graphics/renderer_2D.hpp"
  ],
  strip_include_prefix="include",
  deps=[
    "//core/common:expected",
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
    "src/renderer_2D.cpp",
  ] + graphics_impl__renderer__opengl_debug_selector,
  deps=[":graphics_impl__renderer__opengl_internal", ":graphics_impl__renderer__hdrs"],
  visibility=["//visibility:private"]
)




alias(
  name="graphics",
  actual=":graphics_impl__renderer__opengl",
  visibility=["//visibility:public"]
)
