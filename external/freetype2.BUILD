package(default_visibility=["//visibility:public"])

cc_library(
    name="freetype2_build",
    hdrs=["include/freetype2/ft2build.h"],
    srcs=["lib/x86_64-linux-gnu/libfreetype.so"],
    strip_include_prefix="include/freetype2",
    visibility=["//visibility:private"]
)

cc_library(
    name="freetype_headers",
    hdrs=glob(["include/freetype2/freetype/**/**"]),
    strip_include_prefix="include/freetype2",
    deps=[":freetype2_build"],
    visibility=["//visibility:private"]
)

cc_library(
    name="freetype2",
    srcs=["lib/x86_64-linux-gnu/libfreetype.so"],
    deps=[":freetype_headers", ":freetype2_build"],
    strip_include_prefix="include",
    visibility=["//visibility:public"]
)
