package(default_visibility=["//visibility:public"])

cc_library(
    name="freetype2",
    hdrs=["include/freetype2/ft2build.h"] + glob(["include/freetype2/freetype/**/**"]),
    srcs=["lib/x86_64-linux-gnu/libfreetype.so"],
    strip_include_prefix="include",
)
