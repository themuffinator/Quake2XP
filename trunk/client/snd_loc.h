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
#include "../win32/winquake.h"
#endif


/*
 =======================================================================

 OpenAL framework

 =======================================================================
*/

#define VER_EAX_NONE	0
#define VER_EFX			1
#define VER_EAX_2		2
#define VER_EAX_3		3
#define VER_EAX_4		4
#define VER_EAX_5		5
#define VER_I3DL2		6

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

//ancient architecture support (other hardware chips and workaround tricks)
#include "AL/eax.h"
#include "AL/3dl2.h"

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

extern LPALEAXSET alEAXSet;
extern LPALEAXGET alEAXGet;
extern I3DL2Get I3dl2Get;
extern I3DL2Set I3dl2Set;


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
typedef ALboolean(AL_APIENTRY * LPEAXSETBUFFERMODE) (ALsizei n,
												 ALuint * buffers,
												 ALint value);
typedef ALenum(AL_APIENTRY * LPEAXGETBUFFERMODE) (ALuint buffer,
											  ALint * value);
extern LPEAXSETBUFFERMODE eaxSetBufferMode;
extern LPEAXGETBUFFERMODE eaxGetBufferMode;
//extern EAXSetBufferMode aleaxSetMode;
//extern EAXGetBufferMode aleaxGetMode;

// Linux has EFX support, so I have replaced _WIN32 checks with _WITH_EAX
#define _WITH_EAX

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
void QAL_Shutdown(void);

#define		MAX_SFX 4096

typedef struct {
	char introName[MAX_QPATH];
	char loopName[MAX_QPATH];
	qboolean looping;
//  fileHandle_t        file;
	FILE *file;
	int start;
	int rate;
	unsigned format;
	void *vorbisFile;
} bgTrack_t;

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
	// OS dependency
#ifdef _WIN32
	HINSTANCE hInstOpenAL;
#else
    //void *hInstOpenAL;
#endif
	unsigned eax;				// EAX version in use.
	unsigned eaxState;			// EAX status

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

typedef struct {
	int rate;
	int width;
	int channels;
	int loopstart;
	int samples;
	int dataofs;				// chunk starts this many bytes from file
								// start
} wavinfo_t;



/*
====================================================================

  SYSTEM SPECIFIC FUNCTIONS

====================================================================
*/
#define MAX_CHANNELS 126		// Creative X-Fi limits (126, except the 1
								// streaming channel)
#define MIN_CHANNELS 13			// NVidia onboard audio. (WIN x64
								// defaults) (13+1)

extern playsound_t s_pendingplays;
extern openal_channel_t s_openal_channels[MAX_CHANNELS];
extern ALuint source_name[MAX_CHANNELS + 1];	// plus 1 streaming
												// channel
extern unsigned s_openal_numChannels;
extern int streaming;

extern cvar_t *s_volume;
extern cvar_t *s_musicvolume;
extern cvar_t *s_show;
extern cvar_t *s_openal_eax;
extern cvar_t *s_openal_device;
extern cvar_t *s_quality;
extern cvar_t *s_distance_model;

#ifdef _WIN32
qboolean alSource_EAX_Flags(ALuint sourceNum, DWORD dwValue);
qboolean alSource_EAX_All(ALuint sourceNum, LPEAXBUFFERPROPERTIES lpData);
#endif

void S_StreamBackgroundTrack(void);
void S_UpdateBackgroundTrack(void);
void S_StartBackgroundTrack(char *introTrack, char *loopTrack);
void S_StopBackgroundTrack(void);
void S_StartCinematic(void);

qboolean StreamingWav_init(char *name);

void S_Play_Wav_Music(void);

void StreamingWav_close(void);

/* willow: !!EXTREMELY BETA!! */
void SCR_audioCinematic(void **cin_data, long *cin_rate, long *ulBytesWritten, ALenum * format);

#endif /* __SND_LOC_H */
