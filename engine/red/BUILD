cc_library(
  name="red_common",
  hdrs=[
    "include/red/components.hpp",
    "include/red/imgui_common.hpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/game",
    "@imgui//:imgui"
  ],
  visibility=["//visibility:private"]
)

cc_binary(
  name="renderer",
  srcs=["src/renderer.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="imgui_start",
  srcs=["src/imgui_start.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="imgui_end",
  srcs=["src/imgui_end.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)