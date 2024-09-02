load("@tyl//:bazel/rules.bzl", "gtest")

cc_library(
  name="script_library_test",
  srcs=[
    "src/script_library_test.cpp",
  ],
  strip_include_prefix="include",
  deps=["//core/game"],
  visibility=["//visibility:private"]
)

cc_library(
  name="component_test",
  srcs=[
    "src/component_test.cpp",
  ],
  strip_include_prefix="include",
  deps=["//core/game"],
  visibility=["//visibility:private"]
)

gtest(
  name="test_component",
  timeout = "short",
  srcs=["test_component.cpp"],
  deps=["//core/game", ":component_test"],
  visibility=["//visibility:public"],
)

gtest(
  name="test_library",
  timeout = "short",
  srcs=["test_library.cpp"],
  deps=["//core/game", ":script_library_test"],
  visibility=["//visibility:public"],
)

gtest(
  name="test_native_script",
  timeout = "short",
  srcs=["test_native_script.cpp"],
  deps=["//core/game", ":script_library_test"],
  visibility=["//visibility:public"],
)
