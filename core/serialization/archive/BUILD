cc_library(
  name="archive",
  hdrs=[
    "include/sde/serial/archive_fwd.hpp",
    "include/sde/serial/iarchive.hpp",
    "include/sde/serial/oarchive.hpp"
  ],
  strip_include_prefix="include",
  deps=["//core/serialization/primitives", "//core/serialization/object"],
  visibility=["//visibility:public"]
)

cc_library(
  name="binary_archive",
  hdrs=[
    "include/sde/serial/binary_archive.hpp",
    "include/sde/serial/binary_iarchive.hpp",
    "include/sde/serial/binary_oarchive.hpp"
  ],
  strip_include_prefix="include",
  deps=[":archive", "//core/serialization/stream"],
  visibility=["//visibility:public"]
)

cc_library(
  name="json_archive",
  hdrs=[
    "include/sde/serial/json_archive.hpp",
    "include/sde/serial/json_iarchive.hpp",
    "include/sde/serial/json_oarchive.hpp"
  ],
  strip_include_prefix="include",
  deps=[":archive", "//core/serialization/stream", "//core/serialization/primitives"],
  visibility=["//visibility:public"]
)
