#ifdef SDE_AUDIO_AL_INL
#error("Detected double include of openal.inl")
#else
#define SDE_AUDIO_AL_INL
#endif  // SDE_AUDIO_AL_INL

// clang-format off

// OpenAL
#include <AL/al.h>
#include <AL/alc.h>

// clang-format on

// C++ Standard Library
#include <type_traits>

// SDE
#include "sde/audio/typedef.hpp"

namespace sde::audio
{

static_assert(std::is_same<ALuint, source_handle_t>());

static_assert(std::is_same<ALuint, buffer_handle_t>());

static inline const char* al_error_to_str(const ALenum error)
{
    switch (error)
    {
        case AL_NO_ERROR: return "AL_NO_ERROR";
        case AL_INVALID_NAME: return "AL_INVALID_NAME";
        case AL_INVALID_ENUM: return "AL_INVALID_ENUM";
        case AL_INVALID_VALUE: return "AL_INVALID_VALUE";
        case AL_INVALID_OPERATION: return "AL_INVALID_OPERATION";
        case AL_OUT_OF_MEMORY: return "AL_OUT_OF_MEMORY";
    }
    return "<<INVALID ERROR CODE>>";
}

}  // sde::audio
