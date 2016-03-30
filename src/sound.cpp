#include "sound.hpp"

using namespace std;

static inline ALenum to_al_format(short channels, short samples)
{
    bool stereo = (channels > 1);

    switch (samples) {
    case 16:
        if (stereo)
            return AL_FORMAT_STEREO16;
        else
            return AL_FORMAT_MONO16;
    case 8:
        if (stereo)
            return AL_FORMAT_STEREO8;
        else
            return AL_FORMAT_MONO8;
        default:
            return -1;
    }
}

// Reference: https://ffainelli.github.io/openal-example/
//
Sound::Sound(string filename)
{
    device = alcOpenDevice(NULL);
    if (!device) {
        fprintf(stderr, "Warning, no device found\n");
        return;
    }

    context = alcCreateContext(device, NULL);
    if (!alcMakeContextCurrent(context)) {
        fprintf(stderr, "Warning, no al context created\n");
        return;
    }

    // All this position stuff doesn't really matter for background music...
    ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
    alListener3f(AL_POSITION, 0, 0, 1.0f);
    alListener3f(AL_VELOCITY, 0, 0, 0);
    alListenerfv(AL_ORIENTATION, listenerOri);

    alGenSources((ALuint)1, &source);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alGenBuffers((ALuint)1, &buffer);

    // TODO: deal with all the memory leaks
    WaveInfo *wave;
    char *bufferData;
    int ret;

    wave = WaveOpenFileForReading(filename.c_str());
    if (!wave) {
        fprintf(stderr, "failed to read wave file\n");
        return;
    }

    ret = WaveSeekFile(0, wave);
    if (ret) {
        fprintf(stderr, "failed to seek wave file\n");
        return;
    }

    bufferData = (char*)malloc(wave->dataSize);

    ret = WaveReadFile(bufferData, wave->dataSize, wave);
    if (ret != wave->dataSize) {
        fprintf(stderr, "short read: %d, want: %d\n", ret, wave->dataSize);
        return;
    }

    alBufferData(buffer, to_al_format(wave->channels, wave->bitsPerSample),
                 bufferData, wave->dataSize, wave->sampleRate);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcePlay(source);
}

Sound::~Sound()
{
    // cleanup context
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);
}
