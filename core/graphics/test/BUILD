load("@tyl//:bazel/rules.bzl", "gtest")

gtest(
  name="texture_io",
  timeout = "short",
  srcs=["texture_io.cpp"],
  deps=["//core/graphics"],
  visibility=["//visibility:public"],
)

gtest(
  name="tile_map_io",
  timeout = "short",
  srcs=["tile_map_io.cpp"],
  deps=["//core/graphics", "//core/serialization/std"],
  visibility=["//visibility:public"],
)

gtest(
  name="sprite_io",
  timeout = "short",
  srcs=["sprite_io.cpp"],
  deps=["//core/graphics"],
  visibility=["//visibility:public"],
)

cc_binary(
    name="playground",
    srcs=["playground.cpp"],
    deps=[
        "//core/graphics:graphics",
        "//core/graphics:window"
    ],
    visibility=["//visibility:public"],
    data=[]
)