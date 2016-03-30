#pragma once

#include <string>

#include "openal/al.h"
#include "openal/alc.h"
#include <audio/wave.h>

class Sound
{
public:
    Sound(std::string filename);
    ~Sound();

private:
    ALCdevice *device;
    ALuint source;
    ALCcontext *context;
    ALuint buffer;
};
