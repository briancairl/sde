cc_library(
  name="build",
  hdrs=[
    "include/sde/build.hpp",
  ],
  strip_include_prefix="include",
  deps=[],
  visibility=["//visibility:public"]
)

cc_library(
  name="expected",
  hdrs=["include/sde/expected.hpp"],
  strip_include_prefix="include",
  deps=["@tl_expected//:tl_expected"],
  visibility=["//visibility:public"]
)

cc_library(
  name="geometry",
  hdrs=["include/sde/geometry_types.hpp", "include/sde/geometry_utils.hpp"],
  srcs=["src/geometry_types.cpp"],
  strip_include_prefix="include",
  deps=["@eigen//:eigen"],
  visibility=["//visibility:public"]
)

cc_library(
  name="logging",
  hdrs=["include/sde/logging.hpp"],
  strip_include_prefix="include",
  deps=[":build"],
  visibility=["//visibility:public"]
)

cc_library(
  name="asset",
  hdrs=["include/sde/asset.hpp"],
  strip_include_prefix="include",
  deps=[],
  visibility=["//visibility:public"]
)

cc_library(
  name="resource",
  hdrs=[
    "include/sde/resource_cache.hpp",
    "include/sde/resource_cache_from_disk.hpp",
    "include/sde/resource_handle.hpp",
    "include/sde/resource_wrapper.hpp",
  ],
  strip_include_prefix="include",
  deps=[":core"],
  visibility=["//visibility:public"]
)

cc_library(
  name="core",
  hdrs=[
    "include/sde/crtp.hpp",
    "include/sde/view.hpp",
    "include/sde/format.hpp",
    "include/sde/type.hpp",
    "include/sde/time.hpp",
    "include/sde/time_ostream.hpp",
  ],
  strip_include_prefix="include",
  deps=[":build", ":logging"],
  visibility=["//visibility:public"]
)