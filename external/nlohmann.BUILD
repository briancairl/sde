cc_library(
    name="json_detail",
    hdrs=glob(["include/nlohmann/detail/**/**/*.hpp"]),
    strip_include_prefix="include",
    visibility=["//visibility:private"],
)

cc_library(
    name="json_thirdparty",
    hdrs=glob(["include/nlohmann/thirdparty/**/*.hpp"]),
    strip_include_prefix="include",
    visibility=["//visibility:private"],
)

cc_library(
    name="json",
    hdrs=glob(["include/nlohmann/*.hpp"]),
    strip_include_prefix="include",
    deps=[":json_detail", ":json_thirdparty"],
    visibility=["//visibility:public"],
)
