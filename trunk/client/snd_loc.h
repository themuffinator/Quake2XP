/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// snd_loc.h -- private sound functions

#ifndef __SND_LOC_H
#define __SND_LOC_H

#ifdef _WIN32
// For HINSTANCE type in <windows.h>
#include "../win32/winquake.h"
#endif

#define MIN(a,b) ((a)>(b) ? (b) : (a))
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define VectorLength_Squared(v) DotProduct(v,v)
#define clamp(a,b,c)	((a) < (b) ? (b) : (a) > (c) ? (c) : (a))

/*
 =======================================================================

 OpenAL framework

 =======================================================================
 */

//main OpenAL framework (Creative's hardware)
#ifdef _WIN32
#define AL_NO_PROTOTYPES YES
#include "AL/al.h"
#define ALC_NO_PROTOTYPES YES
#include "AL/alc.h"
#include "AL/efx.h"
#include "AL/efx-creative.h"
#include "AL/EFX-Util.h"

#include "AL/xram.h"
#define _WITH_XRAM 0

extern LPALCOPENDEVICE alcOpenDevice;
extern LPALCCLOSEDEVICE alcCloseDevice;
extern LPALCCREATECONTEXT alcCreateContext;
extern LPALCDESTROYCONTEXT alcDestroyContext;
extern LPALCMAKECONTEXTCURRENT alcMakeContextCurrent;
extern LPALCPROCESSCONTEXT alcProcessContext;
extern LPALCSUSPENDCONTEXT alcSuspendContext;
extern LPALCGETCURRENTCONTEXT alcGetCurrentContext;
extern LPALCGETCONTEXTSDEVICE alcGetContextsDevice;
extern LPALCGETSTRING alcGetString;
extern LPALCGETINTEGERV alcGetIntegerv;
extern LPALCGETERROR alcGetError;
extern LPALCISEXTENSIONPRESENT alcIsExtensionPresent;
extern LPALCGETPROCADDRESS alcGetProcAddress;
extern LPALCGETENUMVALUE alcGetEnumValue;

extern LPALBUFFERDATA alBufferData;
extern LPALDELETEBUFFERS alDeleteBuffers;
extern LPALDELETESOURCES alDeleteSources;
extern LPALDISABLE alDisable;
extern LPALDOPPLERFACTOR alDopplerFactor;
extern LPALDOPPLERVELOCITY alDopplerVelocity;
extern LPALSPEEDOFSOUND    alSpeedOfSound;
extern LPALENABLE alEnable;
extern LPALGENBUFFERS alGenBuffers;
extern LPALGETBOOLEAN alGetBoolean;
extern LPALGETBOOLEANV alGetBooleanv;
extern LPALGETBUFFERF alGetBufferf;
extern LPALGETBUFFERI alGetBufferi;
extern LPALGETDOUBLE alGetDouble;
extern LPALGETDOUBLEV alGetDoublev;
extern LPALGETENUMVALUE alGetEnumValue;
extern LPALGETERROR alGetError;
extern LPALGETFLOAT alGetFloat;
extern LPALGETFLOATV alGetFloatv;
extern LPALGETINTEGER alGetInteger;
extern LPALGETINTEGERV alGetIntegerv;
extern LPALGETLISTENER3F alGetListener3f;
extern LPALGETLISTENERF alGetListenerf;
extern LPALGETLISTENERFV alGetListenerfv;
extern LPALGETLISTENERI alGetListeneri;
extern LPALGETPROCADDRESS alGetProcAddress;
extern LPALGETSOURCE3F alGetSource3f;
extern LPALGETSOURCEF alGetSourcef;
extern LPALGETSOURCEFV alGetSourcefv;
extern LPALGETSOURCEI alGetSourcei;
extern LPALGETSTRING alGetString;
extern LPALISBUFFER alIsBuffer;
extern LPALISENABLED alIsEnabled;
extern LPALISEXTENSIONPRESENT alIsExtensionPresent;
extern LPALISSOURCE alIsSource;
extern LPALLISTENER3F alListener3f;
extern LPALLISTENERF alListenerf;
extern LPALLISTENERFV alListenerfv;
extern LPALLISTENERI alListeneri;
extern LPALSOURCE3F alSource3f;
extern LPALSOURCEF alSourcef;
extern LPALSOURCEFV alSourcefv;
extern LPALSOURCEI alSourcei;
extern LPALSOURCEPAUSE alSourcePause;
extern LPALSOURCEPAUSEV alSourcePausev;
extern LPALSOURCEPLAY alSourcePlay;
extern LPALSOURCEPLAYV alSourcePlayv;
extern LPALSOURCEQUEUEBUFFERS alSourceQueueBuffers;
extern LPALSOURCEREWIND alSourceRewind;
extern LPALSOURCEREWINDV alSourceRewindv;
extern LPALSOURCESTOP alSourceStop;
extern LPALSOURCESTOPV alSourceStopv;
extern LPALSOURCEUNQUEUEBUFFERS alSourceUnqueueBuffers;
extern LPALGENSOURCES alGenSources;
extern LPALDISTANCEMODEL alDistanceModel;
extern LPALSOURCE3I alSource3i;

// EFX Extension function pointer variables

// Effect objects
extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALISEFFECT alIsEffect;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTIV alEffectiv;
extern LPALEFFECTF alEffectf;
extern LPALEFFECTFV alEffectfv;
extern LPALGETEFFECTI alGetEffecti;
extern LPALGETEFFECTIV alGetEffectiv;
extern LPALGETEFFECTF alGetEffectf;
extern LPALGETEFFECTFV alGetEffectfv;

// Filter objects
extern LPALGENFILTERS alGenFilters;
extern LPALDELETEFILTERS alDeleteFilters;
extern LPALISFILTER alIsFilter;
extern LPALFILTERI alFilteri;
extern LPALFILTERIV alFilteriv;
extern LPALFILTERF alFilterf;
extern LPALFILTERFV alFilterfv;
extern LPALGETFILTERI alGetFilteri;
extern LPALGETFILTERIV alGetFilteriv;
extern LPALGETFILTERF alGetFilterf;
extern LPALGETFILTERFV alGetFilterfv;

// Auxiliary slot object
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;

// XRAM Extension function pointer variables and enum values
typedef ALboolean (AL_APIENTRY * LPEAXSETBUFFERMODE) (ALsizei n,
	ALuint * buffers,
	ALint value);
typedef ALenum (AL_APIENTRY * LPEAXGETBUFFERMODE) (ALuint buffer,
	ALint * value);
extern LPEAXSETBUFFERMODE eaxSetBufferMode;
extern LPEAXGETBUFFERMODE eaxGetBufferMode;

#else
#define AL_ALEXT_PROTOTYPES
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/efx.h>
#endif


/*
 =======================================================================

 IMPLEMENTATION SPECIFIC FUNCTIONS

 =======================================================================
 */
void QAL_Shutdown (void);

#define		MAX_SFX 4096

typedef struct {
	unsigned long flags;		// collection of 1 bit data
	vec3_t _AL_POSITION;

	int entNum;					// To allow overriding a specific sound
	int entChannel;
	ALuint bufferNum;			// willow: "0" is an undocumented "free
	// state" descriptor, i believe.

	// int startTime; // For overriding oldest sounds
	// float distanceMult; //willow: It's wrong!
} openal_channel_t;

typedef struct {
	// In Linux we link at compile time
#ifdef _WIN32
	HINSTANCE hInstOpenAL;
#endif
	qboolean efx;

	// OpenAL internals
	ALCdevice *hDevice;
	ALCcontext *hALC;
	unsigned device_count;
} alConfig_t;

extern alConfig_t alConfig;
extern qboolean openalStop;

// a playsound_t will be generated if game engine delayed the start of sample.
typedef struct playsound_s {
	struct playsound_s *prev, *next;
	ALuint bufferNum;
	float volume;
	float attenuation;
	int entnum;
	int entchannel;
	qboolean fixed_origin;		// use origin field instead of entnum's
	// origin
	vec3_t origin;
	vec3_t velocity;			// willow: TO DO!
	unsigned begin;				// begin on this sample
} playsound_t;


/*
====================================================================

SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/

#define MAX_CHANNELS 126		// Creative X-Fi limits (126, except the 1
// streaming channel)
#define MIN_CHANNELS 13			// NVidia onboard audio. (WIN x64
// defaults) (13+1)

#define CH_STREAMING s_openal_numChannels

extern playsound_t s_pendingplays;
extern openal_channel_t s_openal_channels[MAX_CHANNELS];
extern ALuint source_name[MAX_CHANNELS + 1];	// plus 1 streaming channel
extern unsigned s_openal_numChannels;

extern cvar_t *s_volume;
extern cvar_t *s_musicvolume;
extern cvar_t *s_musicsrc;
extern cvar_t *s_musicrandom;
extern cvar_t *s_show;
extern cvar_t *s_openal_efx;
extern cvar_t *s_openal_device;
extern cvar_t *s_quality;
extern cvar_t *s_distance_model;

void EFX_RvbInit (void);
void EFX_RvbUpdate (vec3_t listener_position);
void EFX_RvbProcSrc (openal_channel_t *ch, ALuint source, qboolean enabled);
void EFX_RvbShutdown (void);

// Streaming and music definitions

#define NUM_STRBUF 8
#define MAX_STRBUF_SIZE (1024*256)

typedef struct {
	// willow: If enabled (not zero) one channel dedicated to cinematic or VOIP communications.
	qboolean enabled;

	// use a buffer queue to mirror OpenAL behavior
	ALuint buffers[NUM_STRBUF];
	unsigned bFirst, bNumAvail;

	ALsizei sound_rate;
	ALenum sound_format;
} streaming_t;
streaming_t streaming;

#define MUSIC_BUFFER_READ_SIZE   4096
byte music_buffer[MAX_STRBUF_SIZE + MUSIC_BUFFER_READ_SIZE];       /// добавка памяти для предотвращения порчи буфера

qboolean S_Streaming_Start (int num_bits, int num_channels, ALsizei rate, float volume);
int S_Streaming_Add (const byte *buffer, int num_bytes);
int S_Streaming_NumFreeBufs (void);
void S_Streaming_Stop (void);

typedef enum {
	MUSIC_NONE, MUSIC_CD, MUSIC_CD_FILES, MUSIC_OTHER_FILES
} music_type_t;

void Music_Init (void);
void Music_Shutdown (void);
void Music_Play (void);
void Music_Stop (void);
void Music_Pause (void);
void Music_Resume (void);
void Music_Update (void);

qboolean S_LoadWAV (const char *name, byte **oWav, byte **oStart, int *oBits, int *oChans, int *oRate, int *oSize);

#endif /* __SND_LOC_H */
