cc_library(
  name="stream",
  hdrs=["include/sde/serial/stream_fwd.hpp", "include/sde/serial/istream.hpp", "include/sde/serial/ostream.hpp"],
  strip_include_prefix="include",
  deps=["//core/common:core"],
  visibility=["//visibility:public"]
)

cc_library(
  name="file_stream",
  hdrs=[
    "include/sde/serial/file_istream.hpp",
    "include/sde/serial/file_ostream.hpp",
    "include/sde/serial/file_stream.hpp",
    "include/sde/serial/file_stream_error.hpp"
  ],
  srcs=[
    "src/file_istream.cpp",
    "src/file_ostream.cpp",
    "src/file_stream_error.cpp",
  ],
  strip_include_prefix="include",
  deps=[":stream", "//core/common:expected"],
  visibility=["//visibility:public"]
)

cc_library(
  name="mem_stream",
  hdrs=[
    "include/sde/serial/mem_istream.hpp",
    "include/sde/serial/mem_ostream.hpp",
    "include/sde/serial/mem_stream.hpp"
  ],
  srcs=[
    "src/mem_istream.cpp",
    "src/mem_ostream.cpp"
  ],
  strip_include_prefix="include",
  deps=[":stream"],
  visibility=["//visibility:public"]
)
