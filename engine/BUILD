BASE_COPTS = []
BASE_LINKOPTS = ["-lstdc++fs", "-lopenal", "-laudio"]

config_setting(
    name = "debug",
    values = {"compilation_mode": "dbg"},
)

config_setting(
    name = "sanitize_only",
    values = {"compilation_mode": "fastbuild"},
)

MODE_COPTS = select({
    ":debug": BASE_COPTS, #+ ["-fsanitize=address", "-fsanitize-address-use-after-scope", "-DADDRESS_SANITIZER", "-g", "-fno-omit-frame-pointer", "-O0"],
    ":sanitize_only": BASE_COPTS + ["-fsanitize=address", "-DADDRESS_SANITIZER", "-g", "-fno-omit-frame-pointer", "-O1", "-DNDEBUG"],
    "//conditions:default": BASE_COPTS + ["-O3", "-DNDEBUG"],
})

MODE_LINKOPTS = select({
    ":debug": BASE_LINKOPTS, #+ ["-fsanitize=address"],
    ":sanitize_only": BASE_LINKOPTS + ["-fsanitize=address"],
    "//conditions:default": BASE_LINKOPTS + [],
})

cc_binary(
    name="engine",
    srcs=["main.cpp"],
    deps=[
        "//core/game",
        "//engine/red:imgui_start",
        "//engine/red:imgui_end",
        "//engine/red:renderer"
    ],
    visibility=["//visibility:public"],
    linkopts=MODE_LINKOPTS,
    copts=MODE_COPTS,
    data=[]
)