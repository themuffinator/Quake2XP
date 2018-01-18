/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/
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
/*
willow - TO PORT MAKERS:
This is windows specific implementation. Link your binary with appropriate functions for linux
in certain folder.
*/
#include "../client/client.h"
#include "../client/snd_loc.h"

/*
willow:
NVIDIA seems to have screwed name. Some sheets referenced to "NVIDIA® nForce(TM) Audio" but
i got "NVIDIA(R) nForce(TM) Audio" instead. For some purpose NVIDIA recommends to reference
"NVIDIA" substring.
*/
#define NVIDIA_DEVICE_NAME      "NVIDIA(R) nForce(TM) Audio"


LPALCOPENDEVICE				alcOpenDevice;
LPALCCLOSEDEVICE			alcCloseDevice;
LPALCCREATECONTEXT			alcCreateContext;
LPALCDESTROYCONTEXT			alcDestroyContext;
LPALCMAKECONTEXTCURRENT		alcMakeContextCurrent;
LPALCPROCESSCONTEXT			alcProcessContext;
LPALCSUSPENDCONTEXT			alcSuspendContext;
LPALCGETCURRENTCONTEXT		alcGetCurrentContext;
LPALCGETCONTEXTSDEVICE		alcGetContextsDevice;
LPALCGETSTRING				alcGetString;
LPALCGETINTEGERV			alcGetIntegerv;
LPALCGETERROR				alcGetError;
LPALCISEXTENSIONPRESENT		alcIsExtensionPresent;
LPALCGETPROCADDRESS			alcGetProcAddress;
LPALCGETENUMVALUE			alcGetEnumValue;

LPALBUFFERDATA				alBufferData;
LPALDELETEBUFFERS			alDeleteBuffers;
LPALDELETESOURCES			alDeleteSources;
LPALDISABLE					alDisable;
LPALDISTANCEMODEL			alDistanceModel;
LPALDOPPLERFACTOR			alDopplerFactor;
LPALDOPPLERVELOCITY			alDopplerVelocity;			//willow:		must not be used in OpenAL 1.1
LPALSPEEDOFSOUND            alSpeedOfSound;
LPALENABLE					alEnable;
LPALGENBUFFERS				alGenBuffers;
LPALGENSOURCES				alGenSources;
LPALGETBOOLEAN				alGetBoolean;
LPALGETBOOLEANV				alGetBooleanv;
LPALGETBUFFERF				alGetBufferf;
LPALGETBUFFERI				alGetBufferi;
LPALGETDOUBLE				alGetDouble;
LPALGETDOUBLEV				alGetDoublev;
LPALGETENUMVALUE			alGetEnumValue;
LPALGETERROR				alGetError;
LPALGETFLOAT				alGetFloat;
LPALGETFLOATV				alGetFloatv;
LPALGETINTEGER				alGetInteger;
LPALGETINTEGERV				alGetIntegerv;
LPALGETLISTENER3F			alGetListener3f;
LPALGETLISTENERF			alGetListenerf;
LPALGETLISTENERFV			alGetListenerfv;
LPALGETLISTENERI			alGetListeneri;
LPALGETPROCADDRESS			alGetProcAddress;
LPALGETSOURCE3F				alGetSource3f;
LPALGETSOURCEF				alGetSourcef;
LPALGETSOURCEFV				alGetSourcefv;
LPALGETSOURCEI				alGetSourcei;
LPALGETSTRING				alGetString;
//LPALHINT					alHint;						willow:		i don't know this extension, neither need it.
LPALISBUFFER				alIsBuffer;
LPALISENABLED				alIsEnabled;
LPALISEXTENSIONPRESENT		alIsExtensionPresent;
LPALISSOURCE				alIsSource;
LPALLISTENER3F				alListener3f;
LPALLISTENERF				alListenerf;
LPALLISTENERFV				alListenerfv;
LPALLISTENERI				alListeneri;
LPALSOURCE3F				alSource3f;
LPALSOURCE3I				alSource3i;
LPALSOURCEF					alSourcef;
LPALSOURCEFV				alSourcefv;
LPALSOURCEI					alSourcei;
LPALSOURCEPAUSE				alSourcePause;
LPALSOURCEPAUSEV			alSourcePausev;
LPALSOURCEPLAY				alSourcePlay;
LPALSOURCEPLAYV				alSourcePlayv;
LPALSOURCEQUEUEBUFFERS		alSourceQueueBuffers;
LPALSOURCEREWIND			alSourceRewind;
LPALSOURCEREWINDV			alSourceRewindv;
LPALSOURCESTOP				alSourceStop;
LPALSOURCESTOPV				alSourceStopv;
LPALSOURCEUNQUEUEBUFFERS	alSourceUnqueueBuffers;

// X-RAM
EAXSetBufferMode			eaxSetBufferMode;
EAXGetBufferMode			eaxGetBufferMode;

// Effect objects
LPALGENEFFECTS				alGenEffects;
LPALDELETEEFFECTS			alDeleteEffects;
LPALISEFFECT				alIsEffect;
LPALEFFECTI					alEffecti;
LPALEFFECTIV				alEffectiv;
LPALEFFECTF					alEffectf;
LPALEFFECTFV				alEffectfv;
LPALGETEFFECTI				alGetEffecti;
LPALGETEFFECTIV				alGetEffectiv;
LPALGETEFFECTF				alGetEffectf;
LPALGETEFFECTFV				alGetEffectfv;

//Filter objects
LPALGENFILTERS				alGenFilters;
LPALDELETEFILTERS			alDeleteFilters;
LPALISFILTER				alIsFilter;
LPALFILTERI					alFilteri;
LPALFILTERIV				alFilteriv;
LPALFILTERF					alFilterf;
LPALFILTERFV				alFilterfv;
LPALGETFILTERI				alGetFilteri;
LPALGETFILTERIV				alGetFilteriv;
LPALGETFILTERF				alGetFilterf;
LPALGETFILTERFV				alGetFilterfv;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS		alGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS	alDeleteAuxiliaryEffectSlots;
LPALISAUXILIARYEFFECTSLOT		alIsAuxiliaryEffectSlot;
LPALAUXILIARYEFFECTSLOTI		alAuxiliaryEffectSloti;
LPALAUXILIARYEFFECTSLOTIV		alAuxiliaryEffectSlotiv;
LPALAUXILIARYEFFECTSLOTF		alAuxiliaryEffectSlotf;
LPALAUXILIARYEFFECTSLOTFV		alAuxiliaryEffectSlotfv;
LPALGETAUXILIARYEFFECTSLOTI		alGetAuxiliaryEffectSloti;
LPALGETAUXILIARYEFFECTSLOTIV	alGetAuxiliaryEffectSlotiv;
LPALGETAUXILIARYEFFECTSLOTF		alGetAuxiliaryEffectSlotf;
LPALGETAUXILIARYEFFECTSLOTFV	alGetAuxiliaryEffectSlotfv;

// =====================================================================

#define GPA(a)			GetProcAddress(alConfig.hInstOpenAL, a);


/*
 =================
 QAL_Shutdown

 Unloads the specified DLL then nulls out all the proc pointers
 =================
 */
void QAL_Shutdown (void) {
	Com_Printf ("...shutting down QAL\n");

	if (alConfig.hInstOpenAL) {
		Com_Printf ("...unloading OpenAL DLL\n");
		FreeLibrary (alConfig.hInstOpenAL);
	}
	memset (&alConfig, 0, sizeof(alConfig_t));

	alcOpenDevice = NULL;
	alcCloseDevice = NULL;
	alcCreateContext = NULL;
	alcDestroyContext = NULL;
	alcMakeContextCurrent = NULL;
	alcProcessContext = NULL;
	alcSuspendContext = NULL;
	alcGetCurrentContext = NULL;
	alcGetContextsDevice = NULL;
	alcGetString = NULL;
	alcGetIntegerv = NULL;
	alcGetError = NULL;
	alcIsExtensionPresent = NULL;
	alcGetProcAddress = NULL;
	alcGetEnumValue = NULL;
	alSource3i = NULL;
	alBufferData = NULL;
	alDeleteBuffers = NULL;
	alDeleteSources = NULL;
	alDistanceModel = NULL;
	alDisable = NULL;
	alDopplerFactor = NULL;
	alDopplerVelocity = NULL;
	alSpeedOfSound = NULL;
	alEnable = NULL;
	alGenBuffers = NULL;
	alGenSources = NULL;
	alGetBoolean = NULL;
	alGetBooleanv = NULL;
	alGetBufferf = NULL;
	alGetBufferi = NULL;
	alGetDouble = NULL;
	alGetDoublev = NULL;
	alGetEnumValue = NULL;
	alGetError = NULL;
	alGetFloat = NULL;
	alGetFloatv = NULL;
	alGetInteger = NULL;
	alGetIntegerv = NULL;
	alGetListener3f = NULL;
	alGetListenerf = NULL;
	alGetListenerfv = NULL;
	alGetListeneri = NULL;
	alGetProcAddress = NULL;
	alGetSource3f = NULL;
	alGetSourcef = NULL;
	alGetSourcefv = NULL;
	alGetSourcei = NULL;
	alGetString = NULL;
	alIsBuffer = NULL;
	alIsEnabled = NULL;
	alIsExtensionPresent = NULL;
	alIsSource = NULL;
	alListener3f = NULL;
	alListenerf = NULL;
	alListenerfv = NULL;
	alListeneri = NULL;
	alSource3f = NULL;
	alSourcef = NULL;
	alSourcefv = NULL;
	alSourcei = NULL;
	alSourcePause = NULL;
	alSourcePausev = NULL;
	alSourcePlay = NULL;
	alSourcePlayv = NULL;
	alSourceQueueBuffers = NULL;
	alSourceRewind = NULL;
	alSourceRewindv = NULL;
	alSourceStop = NULL;
	alSourceStopv = NULL;
	alSourceUnqueueBuffers = NULL;

	// Effect objects
	alGenEffects = NULL;
	alDeleteEffects = NULL;
	alIsEffect = NULL;
	alEffecti = NULL;
	alEffectiv = NULL;
	alEffectf = NULL;
	alEffectfv = NULL;
	alGetEffecti = NULL;
	alGetEffectiv = NULL;
	alGetEffectf = NULL;
	alGetEffectfv = NULL;

	//Filter objects
	alGenFilters = NULL;
	alDeleteFilters = NULL;
	alIsFilter = NULL;
	alFilteri = NULL;
	alFilteriv = NULL;
	alFilterf = NULL;
	alFilterfv = NULL;
	alGetFilteri = NULL;
	alGetFilteriv = NULL;
	alGetFilterf = NULL;
	alGetFilterfv = NULL;

	// Auxiliary slot object
	alGenAuxiliaryEffectSlots = NULL;
	alDeleteAuxiliaryEffectSlots = NULL;
	alIsAuxiliaryEffectSlot = NULL;
	alAuxiliaryEffectSloti = NULL;
	alAuxiliaryEffectSlotiv = NULL;
	alAuxiliaryEffectSlotf = NULL;
	alAuxiliaryEffectSlotfv = NULL;
	alGetAuxiliaryEffectSloti = NULL;
	alGetAuxiliaryEffectSlotiv = NULL;
	alGetAuxiliaryEffectSlotf = NULL;
	alGetAuxiliaryEffectSlotfv = NULL;
}

void S_SoundInfo_f (void);

ALboolean EAXSetBufferMode_NULL (ALsizei n, ALuint *buffers, ALint value) {
	//willow:
	//Always successfull, we do not really care of non X-RAM audio chips.
	//No X-RAM no deal no failures too.
	return AL_TRUE;
}

ALenum EAXGetBufferMode_NULL (ALuint buffer, ALint *value) {
	//willow:
	//I'm not interested in this function anyway, just heck it would be existed and even realistic!
	return alGetEnumValue ("AL_STORAGE_AUTOMATIC");
}

/*
 =================
 AL_Init
 =================
 */
qboolean AL_StartOpenAL (void);

qboolean AL_Init (int hardreset) {
	if (hardreset) {
		char	path[MAX_OSPATH];
		Com_Printf ("\n");
		Com_Printf ("==="S_COLOR_YELLOW"Starting OpenAL audio subsystem"S_COLOR_WHITE"===\n");
		Com_Printf ("\n");

		if (!SearchPath (NULL, "OpenAL32.dll", NULL, sizeof(path), path, NULL)) {
			Sys_Error("...ERROR: couldn't find OpenAL driver 'OpenAL32.dll'\n");
			openalStop = qtrue;
			return qfalse;
		}

		Com_DPrintf ("...LoadLibrary( '%s' ) \n", path);
		if ((alConfig.hInstOpenAL = LoadLibrary (path)) == NULL) {
			Sys_Error("...ERROR: couldn't load OpenAL driver 'OpenAL32.dll'\n");
			openalStop = qtrue;
			return qfalse;
		}

		// Binds our AL function pointers to the appropriate AL stuff
		alcOpenDevice = (LPALCOPENDEVICE)GPA ("alcOpenDevice");
		alcCloseDevice = (LPALCCLOSEDEVICE)GPA ("alcCloseDevice");
		alcCreateContext = (LPALCCREATECONTEXT)GPA ("alcCreateContext");
		alcDestroyContext = (LPALCDESTROYCONTEXT)GPA ("alcDestroyContext");
		alcMakeContextCurrent = (LPALCMAKECONTEXTCURRENT)GPA ("alcMakeContextCurrent");
		alcProcessContext = (LPALCPROCESSCONTEXT)GPA ("alcProcessContext");
		alcSuspendContext = (LPALCSUSPENDCONTEXT)GPA ("alcSuspendContext");
		alcGetCurrentContext = (LPALCGETCURRENTCONTEXT)GPA ("alcGetCurrentContext");
		alcGetContextsDevice = (LPALCGETCONTEXTSDEVICE)GPA ("alcGetContextsDevice");
		alcGetString = (LPALCGETSTRING)GPA ("alcGetString");
		alcGetIntegerv = (LPALCGETINTEGERV)GPA ("alcGetIntegerv");
		alcGetError = (LPALCGETERROR)GPA ("alcGetError");
		alcIsExtensionPresent = (LPALCISEXTENSIONPRESENT)GPA ("alcIsExtensionPresent");
		alcGetProcAddress = (LPALCGETPROCADDRESS)GPA ("alcGetProcAddress");
		alcGetEnumValue = (LPALCGETENUMVALUE)GPA ("alcGetEnumValue");

		alBufferData = (LPALBUFFERDATA)GPA ("alBufferData");
		alDeleteBuffers = (LPALDELETEBUFFERS)GPA ("alDeleteBuffers");
		alDeleteSources = (LPALDELETESOURCES)GPA ("alDeleteSources");

		alDistanceModel = (LPALDISTANCEMODEL)GPA ("alDistanceModel");
		alDisable = (LPALDISABLE)GPA ("alDisable");
		alDopplerFactor = (LPALDOPPLERFACTOR)GPA ("alDopplerFactor");
		alDopplerVelocity = (LPALDOPPLERVELOCITY)GPA ("alDopplerVelocity");
		alSpeedOfSound = (LPALSPEEDOFSOUND)GPA ("alSpeedOfSound");
		alEnable = (LPALENABLE)GPA ("alEnable");
		alGenBuffers = (LPALGENBUFFERS)GPA ("alGenBuffers");
		alGenSources = (LPALGENSOURCES)GPA ("alGenSources");
		alGetBoolean = (LPALGETBOOLEAN)GPA ("alGetBoolean");
		alGetBooleanv = (LPALGETBOOLEANV)GPA ("alGetBooleanv");
		alGetBufferf = (LPALGETBUFFERF)GPA ("alGetBufferf");
		alGetBufferi = (LPALGETBUFFERI)GPA ("alGetBufferi");
		alGetDouble = (LPALGETDOUBLE)GPA ("alGetDouble");
		alGetDoublev = (LPALGETDOUBLEV)GPA ("alGetDoublev");
		alGetEnumValue = (LPALGETENUMVALUE)GPA ("alGetEnumValue");
		alGetError = (LPALGETERROR)GPA ("alGetError");
		alGetFloat = (LPALGETFLOAT)GPA ("alGetFloat");
		alGetFloatv = (LPALGETFLOATV)GPA ("alGetFloatv");
		alGetInteger = (LPALGETINTEGER)GPA ("alGetInteger");
		alGetIntegerv = (LPALGETINTEGERV)GPA ("alGetIntegerv");
		alGetListener3f = (LPALGETLISTENER3F)GPA ("alGetListener3f");
		alGetListenerf = (LPALGETLISTENERF)GPA ("alGetListenerf");
		alGetListenerfv = (LPALGETLISTENERFV)GPA ("alGetListenerfv");
		alGetListeneri = (LPALGETLISTENERI)GPA ("alGetListeneri");
		alGetProcAddress = (LPALGETPROCADDRESS)GPA ("alGetProcAddress");
		alGetSource3f = (LPALGETSOURCE3F)GPA ("alGetSource3f");
		alGetSourcef = (LPALGETSOURCEF)GPA ("alGetSourcef");
		alGetSourcefv = (LPALGETSOURCEFV)GPA ("alGetSourcefv");
		alGetSourcei = (LPALGETSOURCEI)GPA ("alGetSourcei");
		alGetString = (LPALGETSTRING)GPA ("alGetString");
		alIsBuffer = (LPALISBUFFER)GPA ("alIsBuffer");
		alIsEnabled = (LPALISENABLED)GPA ("alIsEnabled");
		alIsExtensionPresent = (LPALISEXTENSIONPRESENT)GPA ("alIsExtensionPresent");
		alIsSource = (LPALISSOURCE)GPA ("alIsSource");
		alListener3f = (LPALLISTENER3F)GPA ("alListener3f");
		alListenerf = (LPALLISTENERF)GPA ("alListenerf");
		alListenerfv = (LPALLISTENERFV)GPA ("alListenerfv");
		alListeneri = (LPALLISTENERI)GPA ("alListeneri");
		alSource3f = (LPALSOURCE3F)GPA ("alSource3f");
		alSourcef = (LPALSOURCEF)GPA ("alSourcef");
		alSourcefv = (LPALSOURCEFV)GPA ("alSourcefv");
		alSourcei = (LPALSOURCEI)GPA ("alSourcei");
		alSourcePause = (LPALSOURCEPAUSE)GPA ("alSourcePause");
		alSourcePausev = (LPALSOURCEPAUSEV)GPA ("alSourcePausev");
		alSourcePlay = (LPALSOURCEPLAY)GPA ("alSourcePlay");
		alSourcePlayv = (LPALSOURCEPLAYV)GPA ("alSourcePlayv");
		alSourceQueueBuffers = (LPALSOURCEQUEUEBUFFERS)GPA ("alSourceQueueBuffers");
		alSourceRewind = (LPALSOURCEREWIND)GPA ("alSourceRewind");
		alSourceRewindv = (LPALSOURCEREWINDV)GPA ("alSourceRewindv");
		alSourceStop = (LPALSOURCESTOP)GPA ("alSourceStop");
		alSourceStopv = (LPALSOURCESTOPV)GPA ("alSourceStopv");
		alSourceUnqueueBuffers = (LPALSOURCEUNQUEUEBUFFERS)GPA ("alSourceUnqueueBuffers");
		alSource3i = (LPALSOURCE3I)alGetProcAddress ("alSource3i");
	}

	// Initialize OpenAL subsystem
	if (!AL_StartOpenAL ()) {
		// Let the user continue without sound
		Com_Printf (S_COLOR_RED"WARNING: OpenAL initialization failed\n");
		openalStop = qtrue;
		return qfalse;
	}

	// Initialize extensions
	alConfig.efx = qfalse;

	// X-RAM
	if (alIsExtensionPresent ((ALubyte *)"EAX-RAM")) {
		Com_Printf ("X-RAM free %d of total %d bytes\n",
			alGetInteger (alGetEnumValue ("AL_EAX_RAM_FREE")),
			alGetInteger (alGetEnumValue ("AL_EAX_RAM_SIZE"))
			);
		eaxSetBufferMode = (EAXSetBufferMode)alGetProcAddress ("EAXSetBufferMode");
		eaxGetBufferMode = (EAXGetBufferMode)alGetProcAddress ("EAXGetBufferMode");
		if (!eaxSetBufferMode) eaxSetBufferMode = EAXSetBufferMode_NULL;
		if (!eaxGetBufferMode) eaxGetBufferMode = EAXGetBufferMode_NULL;
	}
	else {
		Com_Printf ("...audio chip without onboard RAM.\n");
		eaxSetBufferMode = EAXSetBufferMode_NULL;
		eaxGetBufferMode = EAXGetBufferMode_NULL;
	}

	// Check for ALC Extensions
	if (alcIsExtensionPresent (alConfig.hDevice, "ALC_EXT_CAPTURE") == AL_TRUE)
		Com_Printf ("...capture capabilities.\n");

	// If EFX is enabled, determine if it's available and use it
	if (s_openal_efx->value) {
		if (alcIsExtensionPresent (alConfig.hDevice, "ALC_EXT_EFX") == AL_TRUE) {
			ALuint		uiEffectSlots[128] = { 0 };
			ALuint		uiEffects[1] = { 0 };
			ALuint		uiFilters[1] = { 0 };
			ALint		iEffectSlotsGenerated;
			ALint		iSends;

			alConfig.efx = qtrue;

			// Imported EFX functions; Get function pointers
			// Effect objects
			alGenEffects = (LPALGENEFFECTS)alGetProcAddress ("alGenEffects");
			alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress ("alDeleteEffects");
			alIsEffect = (LPALISEFFECT)alGetProcAddress ("alIsEffect");
			alEffecti = (LPALEFFECTI)alGetProcAddress ("alEffecti");
			alEffectiv = (LPALEFFECTIV)alGetProcAddress ("alEffectiv");
			alEffectf = (LPALEFFECTF)alGetProcAddress ("alEffectf");
			alEffectfv = (LPALEFFECTFV)alGetProcAddress ("alEffectfv");
			alGetEffecti = (LPALGETEFFECTI)alGetProcAddress ("alGetEffecti");
			alGetEffectiv = (LPALGETEFFECTIV)alGetProcAddress ("alGetEffectiv");
			alGetEffectf = (LPALGETEFFECTF)alGetProcAddress ("alGetEffectf");
			alGetEffectfv = (LPALGETEFFECTFV)alGetProcAddress ("alGetEffectfv");
			//Filter objects
			alGenFilters = (LPALGENFILTERS)alGetProcAddress ("alGenFilters");
			alDeleteFilters = (LPALDELETEFILTERS)alGetProcAddress ("alDeleteFilters");
			alIsFilter = (LPALISFILTER)alGetProcAddress ("alIsFilter");
			alFilteri = (LPALFILTERI)alGetProcAddress ("alFilteri");
			alFilteriv = (LPALFILTERIV)alGetProcAddress ("alFilteriv");
			alFilterf = (LPALFILTERF)alGetProcAddress ("alFilterf");
			alFilterfv = (LPALFILTERFV)alGetProcAddress ("alFilterfv");
			alGetFilteri = (LPALGETFILTERI)alGetProcAddress ("alGetFilteri");
			alGetFilteriv = (LPALGETFILTERIV)alGetProcAddress ("alGetFilteriv");
			alGetFilterf = (LPALGETFILTERF)alGetProcAddress ("alGetFilterf");
			alGetFilterfv = (LPALGETFILTERFV)alGetProcAddress ("alGetFilterfv");
			// Auxiliary slot object
			alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress ("alGenAuxiliaryEffectSlots");
			alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress ("alDeleteAuxiliaryEffectSlots");
			alIsAuxiliaryEffectSlot = (LPALISAUXILIARYEFFECTSLOT)alGetProcAddress ("alIsAuxiliaryEffectSlot");
			alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress ("alAuxiliaryEffectSloti");
			alAuxiliaryEffectSlotiv = (LPALAUXILIARYEFFECTSLOTIV)alGetProcAddress ("alAuxiliaryEffectSlotiv");
			alAuxiliaryEffectSlotf = (LPALAUXILIARYEFFECTSLOTF)alGetProcAddress ("alAuxiliaryEffectSlotf");
			alAuxiliaryEffectSlotfv = (LPALAUXILIARYEFFECTSLOTFV)alGetProcAddress ("alAuxiliaryEffectSlotfv");
			alGetAuxiliaryEffectSloti = (LPALGETAUXILIARYEFFECTSLOTI)alGetProcAddress ("alGetAuxiliaryEffectSloti");
			alGetAuxiliaryEffectSlotiv = (LPALGETAUXILIARYEFFECTSLOTIV)alGetProcAddress ("alGetAuxiliaryEffectSlotiv");
			alGetAuxiliaryEffectSlotf = (LPALGETAUXILIARYEFFECTSLOTF)alGetProcAddress ("alGetAuxiliaryEffectSlotf");
			alGetAuxiliaryEffectSlotfv = (LPALGETAUXILIARYEFFECTSLOTFV)alGetProcAddress ("alGetAuxiliaryEffectSlotfv");

			// To determine how many Auxiliary Effects Slots are available, create as many as possible (up to 128)
			// until the call fails.
			for (iEffectSlotsGenerated = 0; iEffectSlotsGenerated < 128; iEffectSlotsGenerated++) {
				alGenAuxiliaryEffectSlots (1, &uiEffectSlots[iEffectSlotsGenerated]);
				if (alGetError () != AL_NO_ERROR)
					break;
			}

			Com_Printf ("\n%d Auxiliary Effect Slot%s\n", iEffectSlotsGenerated, (iEffectSlotsGenerated == 1) ? "" : "s");

			// Retrieve the number of Auxiliary Effect Slots Sends available on each Source
			alcGetIntegerv (alConfig.hDevice, ALC_MAX_AUXILIARY_SENDS, 1, &iSends);
			Com_Printf ("%d Auxiliary Send%s per Source\n", iSends, (iSends == 1) ? "" : "s");

			// To determine which Effects are supported, generate an Effect Object, and try to set its type to
			// the various Effect enum values
			Com_Printf ("\nEffects Supported: -\n");
			alGenEffects (1, &uiEffects[0]);
			if (alGetError () == AL_NO_ERROR) {
				// Try setting Effect Type to known Effects
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_REVERB);
				Com_Printf ("'Reverb' Support            %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
				Com_Printf ("'EAX Reverb' Support        %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_CHORUS);
				Com_Printf ("'Chorus' Support            %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_DISTORTION);
				Com_Printf ("'Distortion' Support        %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_ECHO);
				Com_Printf ("'Echo' Support              %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
				Com_Printf ("'Flanger' Support           %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_FREQUENCY_SHIFTER);
				Com_Printf ("'Frequency Shifter' Support %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_VOCAL_MORPHER);
				Com_Printf ("'Vocal Morpher' Support     %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_PITCH_SHIFTER);
				Com_Printf ("'Pitch Shifter' Support     %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_RING_MODULATOR);
				Com_Printf ("'Ring Modulator' Support    %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_AUTOWAH);
				Com_Printf ("'Autowah' Support           %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_COMPRESSOR);
				Com_Printf ("'Compressor' Support        %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alEffecti (uiEffects[0], AL_EFFECT_TYPE, AL_EFFECT_EQUALIZER);
				Com_Printf ("'Equalizer' Support         %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
			}
			// To determine which Filters are supported, generate a Filter Object, and try to set its type to
			// the various Filter enum values
			Com_Printf ("\nFilters Supported: -\n");
			// Generate a Filter to use to determine what Filter Types are supported
			alGenFilters (1, &uiFilters[0]);
			if (alGetError () == AL_NO_ERROR) {
				// Try setting the Filter type to known Filters
				alFilteri (uiFilters[0], AL_FILTER_TYPE, AL_FILTER_LOWPASS);
				Com_Printf ("'Low Pass'  Support         %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alFilteri (uiFilters[0], AL_FILTER_TYPE, AL_FILTER_HIGHPASS);
				Com_Printf ("'High Pass' Support         %s\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
				alFilteri (uiFilters[0], AL_FILTER_TYPE, AL_FILTER_BANDPASS);
				Com_Printf ("'Band Pass' Support         %s\n\n", (alGetError () == AL_NO_ERROR) ? "YES" : "NO");
			}
			// Clean-up ...
			// Delete Filter
			alDeleteFilters (1, &uiFilters[0]);
			// Delete Effect
			alDeleteEffects (1, &uiEffects[0]);
			// Delete Auxiliary Effect Slots
			alDeleteAuxiliaryEffectSlots (iEffectSlotsGenerated, uiEffectSlots);
		}
	}//<---(!s_openal_efx->value)

	// Check for AL Extensions

	if (alIsExtensionPresent ("AL_EXT_OFFSET") == AL_TRUE)
		Com_Printf ("...sample offset capabilities.\n");
	if (alIsExtensionPresent ("AL_EXT_LINEAR_DISTANCE") == AL_TRUE)
		Com_Printf ("...unlocked linear distance preset.\n");
	if (alIsExtensionPresent ("AL_EXT_EXPONENT_DISTANCE") == AL_TRUE)
		Com_Printf ("...unlocked exponent distance preset.\n");

	return qtrue;
}
