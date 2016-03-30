#ifndef _AL_AUXEFFECTSLOT_H_
#define _AL_AUXEFFECTSLOT_H_

#include "alMain.h"
#include "alEffect.h"

#include "align.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ALeffectStateVtable;
struct ALeffectslot;

typedef struct ALeffectState {
    const struct ALeffectStateVtable *vtbl;

    ALfloat (*OutBuffer)[BUFFERSIZE];
    ALuint OutChannels;
} ALeffectState;

struct ALeffectStateVtable {
    void (*const Destruct)(ALeffectState *state);

    ALboolean (*const deviceUpdate)(ALeffectState *state, ALCdevice *device);
    void (*const update)(ALeffectState *state, const ALCdevice *device, const struct ALeffectslot *slot);
    void (*const process)(ALeffectState *state, ALuint samplesToDo, const ALfloat (*restrict samplesIn)[BUFFERSIZE], ALfloat (*restrict samplesOut)[BUFFERSIZE], ALuint numChannels);

    void (*const Delete)(void *ptr);
};

#define DEFINE_ALEFFECTSTATE_VTABLE(T)                                        \
DECLARE_THUNK(T, ALeffectState, void, Destruct)                               \
DECLARE_THUNK1(T, ALeffectState, ALboolean, deviceUpdate, ALCdevice*)         \
DECLARE_THUNK2(T, ALeffectState, void, update, const ALCdevice*, const ALeffectslot*) \
DECLARE_THUNK4(T, ALeffectState, void, process, ALuint, const ALfloatBUFFERSIZE*restrict, ALfloatBUFFERSIZE*restrict, ALuint) \
static void T##_ALeffectState_Delete(void *ptr)                               \
{ return T##_Delete(STATIC_UPCAST(T, ALeffectState, (ALeffectState*)ptr)); }  \
                                                                              \
static const struct ALeffectStateVtable T##_ALeffectState_vtable = {          \
    T##_ALeffectState_Destruct,                                               \
                                                                              \
    T##_ALeffectState_deviceUpdate,                                           \
    T##_ALeffectState_update,                                                 \
    T##_ALeffectState_process,                                                \
                                                                              \
    T##_ALeffectState_Delete,                                                 \
}


struct ALeffectStateFactoryVtable;

typedef struct ALeffectStateFactory {
    const struct ALeffectStateFactoryVtable *vtbl;
} ALeffectStateFactory;

struct ALeffectStateFactoryVtable {
    ALeffectState *(*const create)(ALeffectStateFactory *factory);
};

#define DEFINE_ALEFFECTSTATEFACTORY_VTABLE(T)                                 \
DECLARE_THUNK(T, ALeffectStateFactory, ALeffectState*, create)                \
                                                                              \
static const struct ALeffectStateFactoryVtable T##_ALeffectStateFactory_vtable = { \
    T##_ALeffectStateFactory_create,                                          \
}


#define MAX_EFFECT_CHANNELS (4)


typedef struct ALeffectslot {
    ALenum EffectType;
    ALeffectProps EffectProps;

    volatile ALfloat   Gain;
    volatile ALboolean AuxSendAuto;

    ATOMIC(ALenum) NeedsUpdate;
    ALeffectState *EffectState;

    RefCount ref;

    /* Self ID */
    ALuint id;

    ALuint NumChannels;
    ChannelConfig AmbiCoeffs[MAX_EFFECT_CHANNELS];
    /* Wet buffer configuration is ACN channel order with N3D scaling:
     * * Channel 0 is the unattenuated mono signal.
     * * Channel 1 is OpenAL -X
     * * Channel 2 is OpenAL Y
     * * Channel 3 is OpenAL -Z
     * Consequently, effects that only want to work with mono input can use
     * channel 0 by itself. Effects that want multichannel can process the
     * ambisonics signal and create a B-Format pan (ComputeBFormatGains) for
     * the device output.
     */
    alignas(16) ALfloat WetBuffer[MAX_EFFECT_CHANNELS][BUFFERSIZE];
} ALeffectslot;

inline struct ALeffectslot *LookupEffectSlot(ALCcontext *context, ALuint id)
{ return (struct ALeffectslot*)LookupUIntMapKey(&context->EffectSlotMap, id); }
inline struct ALeffectslot *RemoveEffectSlot(ALCcontext *context, ALuint id)
{ return (struct ALeffectslot*)RemoveUIntMapKey(&context->EffectSlotMap, id); }

ALenum InitEffectSlot(ALeffectslot *slot);
ALvoid ReleaseALAuxiliaryEffectSlots(ALCcontext *Context);


ALeffectStateFactory *ALnullStateFactory_getFactory(void);
ALeffectStateFactory *ALreverbStateFactory_getFactory(void);
ALeffectStateFactory *ALautowahStateFactory_getFactory(void);
ALeffectStateFactory *ALchorusStateFactory_getFactory(void);
ALeffectStateFactory *ALcompressorStateFactory_getFactory(void);
ALeffectStateFactory *ALdistortionStateFactory_getFactory(void);
ALeffectStateFactory *ALechoStateFactory_getFactory(void);
ALeffectStateFactory *ALequalizerStateFactory_getFactory(void);
ALeffectStateFactory *ALflangerStateFactory_getFactory(void);
ALeffectStateFactory *ALmodulatorStateFactory_getFactory(void);

ALeffectStateFactory *ALdedicatedStateFactory_getFactory(void);


ALenum InitializeEffect(ALCdevice *Device, ALeffectslot *EffectSlot, ALeffect *effect);

void InitEffectFactoryMap(void);
void DeinitEffectFactoryMap(void);

#ifdef __cplusplus
}
#endif

#endif
