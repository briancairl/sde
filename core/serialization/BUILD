cc_library(
  name="serialization",
  hdrs=[
    "serialization.hpp",
    "serialization_fwd.hpp",
    "serialization_binary_file.hpp",
    "serialization_binary_file_fwd.hpp"
  ],
  srcs=[
    "serialization.cc"
  ],
  include_prefix="sde",
  deps=[
    "//core/serialization/archive",
    "//core/serialization/archive:binary_archive",
    "//core/serialization/archive:json_archive",
    "//core/serialization/stream",
    "//core/serialization/stream:file_stream",
    "//core/serialization/stream:mem_stream",
    "//core/serialization/primitives",
    "//core/serialization/object",
  ],
  visibility=["//visibility:public"]
)
