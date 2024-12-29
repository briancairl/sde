cc_library(
  name="archive",
  hdrs=[
    "include/sde/game/archive.hpp",
    "include/sde/game/archive_fwd.hpp",
  ],
  srcs = [],
  strip_include_prefix="include",
  deps=[
    "//core/serialization",
  ],
  visibility=["//visibility:public"]
)


cc_library(
  name="library",
  hdrs=[
    "include/sde/game/library.hpp",
    "include/sde/game/library_fwd.hpp",
    "include/sde/game/library_handle.hpp",
  ],
  srcs = [
    "src/library.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/common:resource",
    "//core/common:stl",
    "//core/dl:export",
    "//core/dl:library",
  ],
  visibility=["//visibility:public"]
)


cc_library(
  name="ecs",
  hdrs=[
    "include/sde/game/component.hpp",
    "include/sde/game/component_decl.hpp",
    "include/sde/game/component_fwd.hpp",
    "include/sde/game/component_handle.hpp",
    "include/sde/game/component_runtime.hpp",
    "include/sde/game/entity.hpp",
    "include/sde/game/entity_fwd.hpp",
    "include/sde/game/entity_handle.hpp",
    "include/sde/game/registry.hpp",
  ],
  srcs = [
    "src/component.cpp",
    "src/entity.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    ":archive",
    ":library",
    "//core/common:resource",
    "//core/common:stl",
    "@entt//:entt",
  ],
  visibility=["//visibility:public"]
)


cc_library(
  name="native_script",
  hdrs=[
    "include/sde/game/native_script.hpp",
    "include/sde/game/native_script_fwd.hpp",
    "include/sde/game/native_script_handle.hpp",
    "include/sde/game/native_script_header.hpp",
    "include/sde/game/native_script_methods.hpp",
    "include/sde/game/native_script_typedefs.hpp",
  ],
  srcs = [
    "src/native_script.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    ":archive",
    ":library",
    "//core/common:resource",
    "//core/common:stl",
  ],
  visibility=["//visibility:public"]
)


cc_library(
  name="native_script_runtime",
  hdrs=[
    "include/sde/game/native_script_runtime.hpp",
    "include/sde/game/native_script_runtime_fwd.hpp",
  ],
  srcs = [],
  strip_include_prefix="include",
  deps=[
    ":archive",
    ":ecs",
    "//core/common:resource",
    "//core/common:stl",
  ],
  visibility=["//visibility:public"]
)


cc_library(
  name="native_script_instance",
  hdrs=[
    "include/sde/game/native_script_instance.hpp",
    "include/sde/game/native_script_instance_fwd.hpp",
    "include/sde/game/native_script_instance_handle.hpp",
  ],
  srcs = [
    "src/native_script_instance.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    ":archive",
    ":native_script",
    "//core/app",
    "//core/common:resource",
    "//core/common:stl",
  ],
  visibility=["//visibility:public"]
)



cc_library(
  name="game",
  hdrs=[
    "include/sde/game/game.hpp",
    "include/sde/game/game_resources.hpp",
    "include/sde/game/game_resources_fwd.hpp",
    "include/sde/game/registry.hpp",
    "include/sde/game/scene.hpp",
    "include/sde/game/scene_fwd.hpp",
    "include/sde/game/scene_graph.hpp",
    "include/sde/game/scene_handle.hpp",
    "include/sde/game/scene_manifest.hpp",
    "include/sde/game/scene_manifest_fwd.hpp",
  ],
  srcs = [
    "src/component.cpp",
    "src/component_preload.cpp",
    "src/entity.cpp",
    "src/game.cpp",
    "src/library.cpp",
    "src/native_script.cpp",
    "src/scene.cpp",
    "src/scene_graph.cpp",
    "src/scene_manifest.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/app",
    "//core/audio",
    "//core/graphics",
    "//core/common:resource",
    "//core/common:stl",
    "//core/dl:export",
    "//core/dl:library",
    "//core/serialization",
    "@dont//:stl",
    "@entt//:entt",
    "@nlohmann//:json",
  ],
  visibility=["//visibility:public"]
)
