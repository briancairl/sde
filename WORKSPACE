workspace(name="tyl")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Boost
# git_repository(
#     name="com_github_nelhage_rules_boost",
#     commit="d104cb7beba996d67ae5826be07aab2d9ca0ee38",
#     remote="https://github.com/nelhage/rules_boost",
#     shallow_since="1637888414 -0800",
# )
# load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")
# boost_deps()

# LibDL
new_local_repository(
    name="libdl",
    build_file="@//external:libdl.BUILD",
    path="/usr/",
)

# Eigen
http_archive(
    name="eigen",
    url="https://github.com/eigenteam/eigen-git-mirror/archive/3.3.4.zip",
    sha256="f5580adc34ea45a4c30200e4100f8a55c55af22b77d4ed05985118fd0b15b77e",
    build_file="eigen.BUILD",
    strip_prefix="eigen-git-mirror-3.3.4",
)

# Entt
git_repository(
  name="entt",
  remote="https://github.com/skypjack/entt.git",
  commit="4a2d1a8541228a90e02a873dba6a980506c42c03",
  shallow_since="1729235740 +0200"
)

# ImGui
new_git_repository(
  name="imgui",
  remote="https://github.com/ocornut/imgui.git",
  commit="2a6d7b1eaa1851e5b53d8c7a361a8d54f5a9ae8c",
  shallow_since="1697651119 +0200",
  build_file="@//external:imgui.BUILD",
)

# ImGui
http_archive(
  name="imgui-file-dialogue",
  url="https://github.com/aiekick/ImGuiFileDialog/archive/refs/heads/Lib_Only.zip",
  sha256="ae827c0cda90581f43f1e832e329fd96f891840fce06eb7adb778af7b8504eae",
  build_file="@//external:imgui_file_dialogue.BUILD",
  strip_prefix="ImGuiFileDialog-Lib_Only"
)

# OpenAL
new_local_repository(
  name="openal",
  path="/usr/",
  build_file="@//external:openal.BUILD",
)

# GLFW
new_local_repository(
  name="glfw",
  path="/usr/",
  build_file="@//external:glfw.BUILD",
)

# FreeType
new_local_repository(
  name="freetype2",
  path="/usr/",
  build_file="@//external:freetype2.BUILD",
)

# STB
new_git_repository(
  name="stb",
  remote="https://github.com/nothings/stb.git",
  commit="b42009b3b9d4ca35bc703f5310eedc74f584be58",
  shallow_since="1594640766 -0700",
  build_file="@//external:stb.BUILD",
)

# TL-expected
new_git_repository(
  name="tl_expected",
  remote="https://github.com/TartanLlama/expected.git",
  commit="aa4f7a5e2422169d48342e369597f463ab666558",
  shallow_since="1678876443 +0000",
  build_file="@//external:tl_expected.BUILD",
)

# JSON
new_git_repository(
  name="nlohmann",
  remote="https://github.com/nlohmann/json.git",
  commit="960b763ecd144f156d05ec61f577b04107290137",
  shallow_since="1720340537 +0200",
  build_file="@//external:nlohmann.BUILD",
)

# DONT
new_git_repository(
  name="dont",
  remote="git@github.com:briancairl/dont.git",
  commit="5c4e3396aa8e597212a80e203a54fc1c52994850",
  shallow_since="1734653584 -0500",
  build_file="@//external:dont.BUILD",
)


# GTest/GMock
http_archive(
    name="googletest",
    url="https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip",
    sha256="1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4",
    strip_prefix="googletest-1.14.0",
    build_file="@//external:googletest.BUILD",
)

## Python ##

git_repository(
    name = "rules_python",
    remote = "https://github.com/bazelbuild/rules_python.git",
    commit = "f40068284cb1074b0205ff6267385a5c987325e2",
    shallow_since = "1658124956 +1000"
)

load("@rules_python//python:repositories.bzl", "python_register_toolchains")

python_register_toolchains(
    name = "python310",
    python_version = "3.10",
)

load("@python310//:defs.bzl", "interpreter")
load("@rules_python//python:pip.bzl", "pip_install")

pip_install(
    python_interpreter_target = interpreter,
    requirements = "//:requirements.txt",
)
