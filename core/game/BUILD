cc_library(
  name="runtime_headers",
  hdrs=[
    "include/sde/game/archive.hpp",
    "include/sde/game/archive_fwd.hpp",
  ],
  srcs = [
  ],
  strip_include_prefix="include",
  deps=[
    "//core/serialization"
  ],
  visibility=["//visibility:private"]
)

cc_library(
  name="library",
  hdrs=[
    "include/sde/game/library.hpp",
    "include/sde/game/library_fwd.hpp",
    "include/sde/game/library_handle.hpp",
  ],
  srcs=[
    "src/library.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/dl:library",
    "//core/common:resource",
  ],
  visibility=["//visibility:public"]
)

cc_library(
  name="component",
  hdrs=[
    "include/sde/game/component_fwd.hpp",
    "include/sde/game/component.hpp",
    "include/sde/game/component_handle.hpp",
    "include/sde/game/component_runtime.hpp",
  ],
  srcs=[
    "src/component.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    ":library",
    ":runtime_headers",
    "//core/dl:export",
    "@entt//:entt",
  ],
  visibility=["//visibility:public"]
)

cc_library(
  name="entity",
  hdrs=[
    "include/sde/game/entity.hpp",
    "include/sde/game/entity_fwd.hpp",
    "include/sde/game/entity_handle.hpp",
  ],
  srcs = [
    "src/entity.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/app",
    "//core/audio",
    "//core/graphics",
    "//core/dl:export",
    "@entt//:entt",
    ":component",
    ":runtime_headers",
  ],
  visibility=["//visibility:private"]
)


cc_library(
  name="runtime",
  hdrs=[
    "include/sde/game/assets.hpp",
    "include/sde/game/assets_fwd.hpp",
  ],
  srcs = [
    "src/assets.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    "//core/app",
    "//core/audio",
    "//core/graphics",
    "//core/dl:export",
    "@entt//:entt",
    ":entity",
    ":component",
    ":runtime_headers",
  ],
  visibility=["//visibility:private"]
)

cc_library(
  name="native_script",
  hdrs=[
    "include/sde/game/native_script_fwd.hpp",
    "include/sde/game/native_script.hpp",
    "include/sde/game/native_script_handle.hpp",
    "include/sde/game/native_script_runtime.hpp",
    "include/sde/game/native_script_runtime_fwd.hpp",
  ],
  srcs=[
    "src/native_script.cpp",
  ],
  strip_include_prefix="include",
  deps=[
    ":library",
    ":runtime",
  ],
  visibility=["//visibility:public"]
)



# cc_library(
#   name="entity",
#   hdrs=[
#     "include/sde/game/entity.hpp",
#     "include/sde/game/entity_fwd.hpp",
#     "include/sde/game/entity_handle.hpp",
#   ],
#   srcs=[
#     "src/entity.cpp",
#   ],
#   strip_include_prefix="include",
#   deps=[
#     "//core/app",
#     "//core/audio",
#     "//core/graphics",
#     "@entt//:entt",
#     "@nlohmann//:json",
#   ],
#   visibility=["//visibility:public"]
# )