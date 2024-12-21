load("@tyl//:bazel/rules.bzl", "gtest")

gtest(
  name="binary_archive",
  timeout = "short",
  srcs=["binary_archive.cpp"],
  deps=["//core/serialization/archive:binary_archive", "//core/serialization/stream:file_stream",],
  visibility=["//visibility:public"],
)
