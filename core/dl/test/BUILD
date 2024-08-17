load("@tyl//:bazel/rules.bzl", "gtest")

cc_library(
  name="libtest",
  hdrs=[
    "include/libtest.h",
  ],
  srcs=[
    "src/libtest.cpp",
  ],
  strip_include_prefix="include",
  visibility=["//visibility:private"]
)

gtest(
  name="dl",
  timeout = "short",
  srcs=["dl.cpp"],
  deps=["//core/dl:library", ":libtest"],
  visibility=["//visibility:public"],
)
