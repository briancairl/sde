cc_library(
  name="core",
  hdrs=[
    "include/sde/crtp.hpp",
    "include/sde/view.hpp"
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
  strip_include_prefix="include",
  deps=["@eigen//:eigen"],
  visibility=["//visibility:public"]
)

cc_library(
  name="logging",
  hdrs=["include/sde/logging.hpp"],
  strip_include_prefix="include",
  deps=[],
  visibility=["//visibility:public"]
)

cc_library(
  name="resources",
  hdrs=["include/sde/resources.hpp"],
  strip_include_prefix="include",
  deps=["@tl_expected//:tl_expected"],
  visibility=["//visibility:public"]
)
