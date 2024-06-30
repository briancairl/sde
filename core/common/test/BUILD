load("@tyl//:bazel/rules.bzl", "gtest")

gtest(
  name="geometry_io",
  timeout = "short",
  srcs=["geometry_io.cpp"],
  deps=["//core/common:geometry"],
  visibility=["//visibility:public"],
)

gtest(
  name="resource_handle_io",
  timeout = "short",
  srcs=["resource_handle_io.cpp"],
  deps=["//core/common:resource"],
  visibility=["//visibility:public"],
)

gtest(
  name="time_io",
  timeout = "short",
  srcs=["time_io.cpp"],
  deps=["//core/common:resource"],
  visibility=["//visibility:public"],
)
