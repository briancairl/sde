### Audio declarations ###

cc_library(
  name="audio_hdrs",
  hdrs=[
    "include/sde/audio/mixer.hpp",
    "include/sde/audio/mixer_fwd.hpp",
    "include/sde/audio/sound.hpp",
    "include/sde/audio/sound_channel_format.hpp",
    "include/sde/audio/sound_data.hpp",
    "include/sde/audio/sound_data_fwd.hpp",
    "include/sde/audio/sound_data_handle.hpp",
    "include/sde/audio/sound_device.hpp",
    "include/sde/audio/sound_device_fwd.hpp",
    "include/sde/audio/sound_fwd.hpp",
    "include/sde/audio/sound_handle.hpp",
  ],
  strip_include_prefix="include",
  visibility=["//visibility:private"]
)

### OpenAL implemenatation ###

cc_library(
  name="audio_impl__openal_user_types",
  hdrs=["include/sde/audio/typedef.hpp"],
  strip_include_prefix="include",
  visibility=["//visibility:private"]
)

cc_library(
  name="audio_impl__openal_internal",
  hdrs=["src/openal.inl"],
  strip_include_prefix="src",
  deps=[
    "//core/common:core",
    "//core/common:expected",
    "//core/common:stl",
    ":audio_impl__openal_user_types",
  ],
  visibility=["//visibility:private"]
)

cc_library(
  name="audio",
  srcs=[
    "src/mixer.cpp",
    "src/sound.cpp",
    "src/sound_channel_format.cpp",
    "src/sound_data.cpp",
    "src/sound_device.cpp",
  ],
  deps=[
    "@openal//:openal",
    "//core/common:asset",
    "//core/common:core",
    "//core/common:expected",
    "//core/common:geometry",
    "//core/common:resource",
    "//core/serialization",
    "//core/serialization/std",
    ":audio_hdrs",
    ":audio_impl__openal_internal"
  ],
  visibility=["//visibility:public"]
)
