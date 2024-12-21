cc_library(
  name="red_common",
  hdrs=[
    "include/red/components.hpp",
    "include/red/imgui_common.hpp",
  ],
  srcs=[
    "src/imgui_common.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/game",
    "@imgui//:imgui"
  ],
  visibility=["//visibility:private"]
)

cc_library(
  name="components_hdrs",
  hdrs=["include/red/components.hpp"],
  strip_include_prefix="include",
  deps=[
    "//core/game",
  ],
  visibility=["//visibility:private"]
)

cc_binary(
  name="components",
  srcs=["src/components.cpp"],
  linkshared=True,
  deps=[":components_hdrs",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="drag_and_drop_loader",
  srcs=["src/drag_and_drop_loader.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
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

cc_binary(
  name="library_browser",
  srcs=["src/library_browser.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="script_browser",
  srcs=["src/script_browser.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="scene_tree",
  srcs=["src/scene_tree.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="texture_viewer",
  srcs=["src/texture_viewer.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="tile_set_editor",
  srcs=["src/tile_set_editor.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="audio_manager",
  srcs=["src/audio_manager.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_binary(
  name="player_character",
  srcs=["src/player_character.cpp"],
  linkshared=True,
  deps=[":red_common",],
  visibility=["//visibility:public"]
)

cc_library(
  name="builtin_native_scripts",
  deps=[
    ":audio_manager",
    ":components",
    ":player_character",
    ":drag_and_drop_loader",
    ":renderer",
    ":imgui_start",
    ":imgui_end",
    ":library_browser",
    ":script_browser",
    ":scene_tree",
    ":texture_viewer",
    ":tile_set_editor",
  ],
  visibility=["//visibility:public"]
)