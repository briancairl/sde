load("@tyl//:bazel/rules.bzl", "gtest")

gtest(
  name="binary_archive",
  timeout = "short",
  srcs=["binary_archive.cpp"],
  deps=["//core/serialization/archive:binary_archive", "//core/serialization/stream:file_stream",],
  visibility=["//visibility:public"],
)

gtest(
  name="hash_archive",
  timeout = "short",
  srcs=["hash_archive.cpp"],
  deps=["//core/serialization/archive:hash_archive" ],
  visibility=["//visibility:public"],
)

gtest(
  name="associative_archive",
  timeout = "short",
  srcs=["associative_archive.cpp"],
  deps=["//core/serialization/archive:binary_archive", "//core/serialization/stream:file_stream", "//core/serialization/archive:associative_archive", "//core/serialization/std"],
  visibility=["//visibility:public"],
)
