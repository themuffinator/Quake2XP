/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2007 willow.

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
// snd_dma.c -- main control for any streaming and OpenAL I/O

#include "client.h"
#include "snd_loc.h"


// =======================================================================
// Internal sound data & structures
// =======================================================================
ALuint source_name[MAX_CHANNELS + 1];

// during registration it is possible to have more sounds
// than could actually be referenced during gameplay,
// because we don't want to free anything until we are
// sure we won't need it.
int num_sfx;
char known_sfx_name[MAX_SFX][MAX_QPATH];
ALuint known_sfx_bufferNum[MAX_SFX];

#define		MAX_PLAYSOUNDS	8
playsound_t s_playsounds[MAX_PLAYSOUNDS];
playsound_t s_freeplays;
playsound_t s_pendingplays;

cvar_t	*s_volume;
cvar_t	*s_show;
cvar_t	*s_musicvolume;
cvar_t	*s_openal_eax;
cvar_t	*s_openal_device;
cvar_t	*s_quality;
cvar_t	*s_distance_model;
cvar_t	*s_initsound;

openal_channel_t s_openal_channels[MAX_CHANNELS];
unsigned s_openal_numChannels;

static struct rbtree *knownsounds;

#ifdef _WITH_EAX
const GUID DSPROPSETID_EAX20_ListenerProperties =
	{ 0x306a6a8, 0xb224, 0x11d2, {0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7,
								  0x22} };
const GUID DSPROPSETID_EAX20_BufferProperties =
	{ 0x306a6a7, 0xb224, 0x11d2, {0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7,
								  0x22} };
#endif

// =======================================================================
// Video & Music streaming
// =======================================================================
int streaming;					// willow: If enabled (not zero) one
								// channel dedicated to cinematic or VOIP
								// communications.
#define			SND_STREAMING_NUMBUFFERS              4
extern byte *stream_wav;
extern long stream_info_rate;
extern long stream_info_samples;
extern long stream_info_dataofs;
ALuint uiBuffers[SND_STREAMING_NUMBUFFERS];
unsigned long ulDataSize = 0;
unsigned long ulBufferSize = 0;
unsigned long ulBytesWritten = 0;
void *pData = NULL;
int BackgroundTrack_Repeat;
void StreamingWav_Reset(void);

/*
=================
Com_Clamp
=================
*/
float Com_Clamp(float value, float min, float max)
{
	return (value < min) ? min : ((value > max) ? max : value);
}

/*
	Customized MATH
	willow's TO DO: add this prototype to common math
*/
float VectorLength_Squared(vec3_t v)
{
	// return DotProduct(v,v);
	return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}

// ====================================================================
// User-setable variables
// ====================================================================

void S_SoundInfo_f(void)
{
	if (!alConfig.hALC) {
		Com_Printf(S_COLOR_RED"Cannot provide OpenAL information\n");
		return;
	}

	Com_Printf("\n");
	Com_Printf("AL_VENDOR:     "S_COLOR_GREEN"%s\n", alGetString(AL_VENDOR));
	Com_Printf("AL_RENDERER:   "S_COLOR_GREEN"%s\n", alGetString(AL_RENDERER));
	Com_Printf("AL_VERSION:    "S_COLOR_GREEN"%s\n", alGetString(AL_VERSION));
	Com_Printf("AL_EXTENSIONS:\n"S_COLOR_GREEN"%s\n", alGetString(AL_EXTENSIONS));

	Com_Printf("\n");
	Com_Printf("DEVICE: "S_COLOR_GREEN"%s\n",
			   alcGetString(alConfig.hDevice, ALC_DEVICE_SPECIFIER));
	Com_Printf("\n");
}

static void AllocChannels(void)
{
#define GPA(a) GetProcAddress(alConfig.hInstOpenAL, a);

#ifdef _WIN32
	LPALGENSOURCES alGenSources = (LPALGENSOURCES) GPA("alGenSources");
#endif

	if (alGenSources) {
		s_openal_numChannels = MAX_CHANNELS + 1;	// +1 streaming
													// channel
		while (s_openal_numChannels > MIN_CHANNELS) {
			alGenSources(s_openal_numChannels, source_name);
			--s_openal_numChannels;
			if (alGetError() == AL_NO_ERROR) {
				Com_Printf("%i mix channels allocated.\n",
						   s_openal_numChannels);
				Com_Printf("streaming channel allocated.\n");
				return;
			}
		}
		Com_Printf("Not enough mix channels!\n");
	}
	s_openal_numChannels = 0;
}

/*
================
S_Init
================
*/
void CL_fast_sound_init(void);
void S_Music_f(void);
void S_Play(void);

void S_Init(int hardreset)
{
	if (hardreset) {
		s_volume			=	Cvar_Get("s_volume", "1", CVAR_ARCHIVE);
		s_show				=	Cvar_Get("s_show", "0", 0);
		s_musicvolume		=	Cvar_Get("s_musicvolume", "0.8", CVAR_ARCHIVE);
		s_openal_device		=	Cvar_Get("s_openal_device", "", CVAR_ARCHIVE);
		s_openal_eax		=	Cvar_Get("s_openal_eax", "0", CVAR_ARCHIVE);
		s_quality			=	Cvar_Get("s_quality", "0", CVAR_ARCHIVE);
		s_distance_model	=	Cvar_Get("s_distance_model", "0", CVAR_ARCHIVE);
		s_initsound			=	Cvar_Get("s_initsound", "1", CVAR_NOSET);	
	}

	if (!s_initsound->value || openalStop)
	{	
		Com_Printf("\n");
		Com_Printf(S_COLOR_YELLOW"=======Sound not initializing.=======\n");
		Com_Printf("\n");
	return;
	}
	else {
		if (AL_Init(hardreset)) {
			AllocChannels();
			if (s_openal_numChannels) {
				S_SoundInfo_f();

				if (s_distance_model->value)	// OpenAL using the best
												// selection by default
				{
					// AL_NONE bypasses all distance
					// attenuation calculation for all sources. The
					// implementation is expected to optimize this
					// situation. AL_INVERSE_DISTANCE_CLAMPED is the IASIG
					// I3DL2 model, with
					// AL_REFERENCE_DISTANCE indicating both the reference
					// distance and the distance
					// below which gain will be clamped.
					// AL_INVERSE_DISTANCE is equivalent to the
					// IASIG I3DL2 model with the exception that
					// AL_REFERENCE_DISTANCE does not
					// imply any clamping. The linear models are not
					// physically realistic, but do allow full
					// attenuation of a source beyond a specified
					// distance. The OpenAL implementation is still
					// free to apply any range clamping as necessary.
					ALenum modelName[7] = {
						AL_NONE,
						AL_INVERSE_DISTANCE,
						AL_INVERSE_DISTANCE_CLAMPED,
						AL_LINEAR_DISTANCE,
						AL_LINEAR_DISTANCE_CLAMPED,
						AL_EXPONENT_DISTANCE,
						AL_EXPONENT_DISTANCE_CLAMPED
					};
#ifdef _WIN32
					LPALDISTANCEMODEL alDistanceModel =
						(LPALDISTANCEMODEL) GPA("alDistanceModel");
#endif
					if (alDistanceModel)
						alDistanceModel(modelName
										[(int) s_distance_model->value -
										 1]);
				}

				alListenerf(AL_GAIN, Com_Clamp(s_volume->value, 0, 1));
				
				alConfig.eaxState = 0xFFFF;	// force EAX state reset

				if (hardreset) {
					Cmd_AddCommand("play", S_Play);
					Cmd_AddCommand("stopsound", S_StopAllSounds);
					Cmd_AddCommand("music", S_Music_f);
					Cmd_AddCommand("s_info", S_SoundInfo_f);
					Cmd_AddCommand("s_test", S_Play_Wav_Music);
					

					CL_fast_sound_init();

					// Generate some AL Buffers for streaming
					alGenBuffers(SND_STREAMING_NUMBUFFERS, uiBuffers);
#ifdef _WITH_EAX
					if (!eaxSetBufferMode
						(SND_STREAMING_NUMBUFFERS, uiBuffers,
						 alGetEnumValue(" AL_STORAGE_ACCESSIBLE"))) {
						// "AL_STORAGE_AUTOMATIC" "AL_STORAGE_HARDWARE"
						// "AL_STORAGE_ACCESSIBLE"
						Com_DPrintf
							("MP3 player: unable to set X-RAM mode\n");
					} 
#endif
					// Streaming memory management
					ulBufferSize = 0x10000;
					pData = malloc(ulBufferSize);
				}

				S_StopAllSounds();	// inits freeplays
			} else
				AL_Shutdown();
		}

		if (!s_openal_numChannels)
		{
			Com_Printf (S_COLOR_RED"OpenAL failed to initialize; no sound available\n");
			Cvar_ForceSet("s_initsound", "0");	/// Berserker's FIX: устранён крэш если OpenAL не может запуститься
			AL_Shutdown();
		}
	}

	Com_Printf("-------------------------------------\n");
}




static void FreeChannels(void)
{
	alDeleteSources(s_openal_numChannels + 1, source_name);
	memset(s_openal_channels, 0, sizeof(openal_channel_t) * MAX_CHANNELS);

	s_openal_numChannels = 0;
}


void FreeSounds(void)
{
	// Stop all sounds
	S_StopAllSounds();

	// Free all sounds
	alDeleteBuffers(num_sfx, known_sfx_bufferNum);

	// Clean up
	memset(known_sfx_name, 0, sizeof(known_sfx_name));
	memset(known_sfx_bufferNum, 0, sizeof(known_sfx_bufferNum));
	num_sfx = 0;
}




// =======================================================================
// Shutdown sound engine
// =======================================================================
void CL_fast_sound_close(void);

void S_Shutdown(void)
{
	if (s_openal_numChannels) {
		// Release temporary streaming storage
		free(pData);
		pData = NULL;

		// Clean up streaming buffers
		alDeleteBuffers(SND_STREAMING_NUMBUFFERS, uiBuffers);

		CL_fast_sound_close();
		FreeSounds();
		
		Cmd_RemoveCommand("play");
		Cmd_RemoveCommand("stopsound");
		Cmd_RemoveCommand("music");
		Cmd_RemoveCommand("s_info");
		Cmd_RemoveCommand("s_test");
		
		FreeChannels();
		AL_Shutdown();
	}
}

// =======================================================================
// Soft reset sound engine
// based on the few ideas:
// 1) Once loaded OpenAL library should be not freed on just context or device change matter.
// 2) Unlike source and listener objects, buffer objects can be shared among AL contexts.
// Buffers are referenced by sources. A single buffer can be referred to by multiple sources.
// This separation allows drivers and hardware to optimize storage and processing where applicable.
// =======================================================================
void S_Restart(void)
{
	S_Shutdown();
	S_Init(1);
}


ALuint S_RegisterSound(const char *name)
{
	if (name[0] == '*')
		return S_RegisterSexedSound(&cl_entities[cl.playernum + 1].current, name);
	else
		return S_FindName((char*)name, s_openal_numChannels);
}


playsound_t *S_AllocPlaysound(void)
{
	playsound_t *ps = s_freeplays.next;

	if (ps == &s_freeplays)
		return NULL;			// no free playsounds

	// unlink from freelist
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	return ps;
}


/*
=================
S_FreePlaysound
=================
*/
void S_FreePlaysound(playsound_t * ps)
{
	// unlink from channel
	ps->prev->next = ps->next;
	ps->next->prev = ps->prev;

	// add to free list
	ps->next = s_freeplays.next;
	s_freeplays.next->prev = ps;
	ps->prev = &s_freeplays;
	s_freeplays.next = ps;
}


/*
=================
S_RegisterSexedSound
=================
*/
extern cvar_t *gender;

ALuint S_RegisterSexedSound(entity_state_t * ent, const char *base)
{
	int n;
	char *p;
	char model[MAX_QPATH];
	char sexedFilename[MAX_QPATH];

	// determine what model the client is using
	model[0] = 0;
	n = CS_PLAYERSKINS + ent->number - 1;
	if (cl.configstrings[n][0]) {
		p = strchr(cl.configstrings[n], '\\');
		if (p) {
			p += 1;
			strcpy(model, p);
			p = strchr(model, '/');
			if (p)
				*p = 0;
		}
	}
	
	// if we can't figure it out, they're male
	if (!model[0]){
		if (!strcmp(cl.clientinfo->sex, "cyborg"))
			strcpy(model, "cyborg");
		else if (!strcmp(cl.clientinfo->sex, "female"))
			strcpy(model, "female");
		else if (!strcmp(cl.clientinfo->sex, "male"))
			strcpy(model, "male");
		else
			strcpy(model, gender->string);
		
	}

	// see if we already know of the model specific sound
	Com_sprintf(sexedFilename, sizeof(sexedFilename), "#players/%s/%s", model, base + 1);

	return S_FindName(sexedFilename, true);

}


//=============================================================================
#define AL_TASK_MANAGER__IS_LOOP_ACTIVE		1
//#define AL_TASK_MANAGER__IS_SOURCE_RELATIVE   2
#define AL_TASK_MANAGER__TERMINATE			4
#define AL_TASK_MANAGER__EXECUTE			8

#define AL_FLAGS_FLAT2D						1
#define AL_FLAGS_AL_LOOPING					2
#define AL_FLAGS_FIXED_POSITION				4	// Use position instead of
												// fetching entity's
												// origin

typedef struct {
	ALfloat TASK_AL_REFERENCE_DISTANCE;
	ALfloat TASK_AL_ROLLOFF_FACTOR;
	ALfloat TASK_AL_GAIN;
	vec3_t TASK_AL_VELOCITY;
	unsigned long flags;
} channel_task_t;

void Flag_set(channel_task_t * task, unsigned long flags_collection)
{
	task->flags |= flags_collection;
}

void Flag_clear(channel_task_t * task, unsigned long flags_collection)
{
	task->flags &= ~flags_collection;
}

qboolean Flag_check(channel_task_t * task, unsigned long flags_collection)
{
	return task->flags & flags_collection;
}

ALuint Flag_checkAL(channel_task_t * task, unsigned long flags_collection)
{
	return task->flags & flags_collection ? AL_TRUE : AL_FALSE;
}

void FlagAL_set(openal_channel_t * ch, unsigned long flags_collection)
{
	ch->flags |= flags_collection;
}

void FlagAL_clear(openal_channel_t * ch, unsigned long flags_collection)
{
	ch->flags &= ~flags_collection;
}

qboolean FlagAL_check(openal_channel_t * ch,
					  unsigned long flags_collection)
{
	return ch->flags & flags_collection;
}

ALuint FlagAL_checkAL(openal_channel_t * ch,
					  unsigned long flags_collection)
{
	return ch->flags & flags_collection ? AL_TRUE : AL_FALSE;
}

void TASK_TerminateChannel(channel_task_t * task, openal_channel_t * ch)
{
	Flag_set(task, AL_TASK_MANAGER__TERMINATE);
	ch->bufferNum = 0;			// ch->sfx = NULL;

//  alSourceStop(ch->sourceNum);
//  alSourcei(ch->sourceNum, AL_BUFFER, 0);
}

// picks a channel based on priorities, empty slots, number of channels
openal_channel_t *PickChannel_NEW(unsigned int entNum,
								  unsigned int entChannel, vec3_t new_vec,
								  ALuint * return_index)
{
	openal_channel_t *ch;
	int i;
	int firstToDie = -1;
//  int                 oldestTime = cl.time;
	qboolean is_terminate = true;

	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		 i++, ch++) {
		// Don't let game sounds override streaming sounds
		/* if (!i && streaming) continue; */

		// Check if this channel is active
/*		if (!ch->sfx)							// found free channel
		{
			firstToDie = i;
			is_terminate = false;
			break;
		} else {*/
		ALuint SourceState;

		alGetSourcei(source_name[i], AL_SOURCE_STATE, &SourceState);

		if (SourceState == AL_STOPPED || SourceState == AL_INITIAL)	// The
																	// source
																	// already
																	// out
																	// of
																	// processing.
		{
			ch->bufferNum = 0;	// ch->sfx = NULL;
			firstToDie = i;
			is_terminate = false;
			break;
		}
	}

	// Emergency channel re-caption.
	// Override sound from same entity willow???????
	/* if (firstToDie == -1) { for (i = 0, ch = s_openal_channels; i <
	   s_openal_numChannels; i++, ch++) if (ch->entNum == entNum &&
	   ch->entChannel == entChannel) { firstToDie = i; break; } } */

	// Emergency channel re-caption. Issue 2
	// Terminate the most distant entity
	if (firstToDie == -1) {
		float len, max;
		vec3_t delta;

		VectorSubtract(cl.refdef.vieworg, new_vec, delta);
		max = VectorLength_Squared(delta);

		for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
			 i++, ch++)
			if (!FlagAL_check(ch, AL_FLAGS_FLAT2D)	// Don't touch FLAT 2D
													// samples!
				&& FlagAL_check(ch, AL_FLAGS_AL_LOOPING)
				) {
				// ch->_AL_POSITION must have same data:
				// alGetSourcefv(ch->sourceNum, AL_POSITION, (ALfloat
				// *)&al_position);
				// willow: i assume cl.refdef.vieworg always have the last
				// listener position
				VectorSubtract(cl.refdef.vieworg, ch->_AL_POSITION, delta);
				// willow:
				// actually we need vector length here, but squared length
				// can do the same job faster.
				len = VectorLength_Squared(delta);
				if (len > max) {
					firstToDie = i;
					max = len;
				}
			}

		if (firstToDie == -1) {
			return NULL;
		}
	}

	ch = &s_openal_channels[firstToDie];

	memcpy(ch->_AL_POSITION, new_vec, sizeof(vec3_t));
	FlagAL_clear(ch, AL_FLAGS_FLAT2D);
	// Flag_clear
	// (&Channels_TODO[firstToDie],AL_TASK_MANAGER__IS_SOURCE_RELATIVE);

	ch->entNum = entNum;
	ch->entChannel = entChannel;
	// ch->startTime = cl.time;

	*return_index = source_name[firstToDie];

	// TERMINATE CHANNEL (stop working channel)
	// TASK_TerminateChannel (&Channels_TODO[firstToDie], ch);
	if (is_terminate) {
		alSourceStop(source_name[firstToDie]);
		// alSourceStop(ch->sourceNum);
		// alSourcei(ch->sourceNum, AL_BUFFER, 0);
	}

	return ch;
}

void S_fastsound(vec3_t origin, int entnum, int entchannel,
				 ALuint bufferNum, ALfloat gain, ALfloat rolloff_factor)
{
	openal_channel_t *ch;
	ALuint sourceNum;

	if (!s_openal_numChannels || !bufferNum)
		return;					// safety check.

	// Pick a channel and start the sound effect
	if (origin)
		ch = PickChannel_NEW(entnum, entchannel, origin, &sourceNum);
	else {
		vec3_t position;
		CL_GetEntityOrigin(entnum, position);
		ch = PickChannel_NEW(entnum, entchannel, position, &sourceNum);
	}

	if (ch) {
#ifdef _WITH_EAX
		EAXBUFFERPROPERTIES normalEAX;
#endif
//      VectorCopy(ps->origin, ch->position);
		ch->bufferNum = bufferNum;	// ch->sfx = sfx;

		// Update min/max distance
		alSourcef(sourceNum, AL_REFERENCE_DISTANCE, 28);
		// Set max distance really far away (default)
		// alSourcef(ch->sourceNum, AL_MAX_DISTANCE, 65536);

		alSourcefv(sourceNum, AL_POSITION, ch->_AL_POSITION);

		if (origin) {
			FlagAL_clear(ch, AL_FLAGS_AL_LOOPING);
			FlagAL_set(ch, AL_FLAGS_FIXED_POSITION);
			alSource3f(sourceNum, AL_VELOCITY, 0, 0, 0);
		} else {
			FlagAL_clear(ch,
						 AL_FLAGS_FIXED_POSITION | AL_FLAGS_AL_LOOPING);
			// willow: TO DO.
			// alSourcefv(sourceNum, AL_VELOCITY,
			// current_task->TASK_AL_VELOCITY);
			alSource3f(sourceNum, AL_VELOCITY, 0, 0, 0);
		}

		alSourcei(sourceNum, AL_BUFFER, bufferNum);
		alSourcei(sourceNum, AL_SOURCE_RELATIVE, AL_FALSE);
		alSourcei(sourceNum, AL_LOOPING,
				  FlagAL_checkAL(ch, AL_FLAGS_AL_LOOPING));
		alSourcef(sourceNum, AL_ROLLOFF_FACTOR, rolloff_factor);
		alSourcef(sourceNum, AL_GAIN, gain);

#ifdef _WITH_EAX
		normalEAX.lDirect = 0;	// direct path level
		normalEAX.lDirectHF = 0;	// direct path level at high
									// frequencies
		normalEAX.lRoom = FlagAL_check(ch, AL_FLAGS_FLAT2D) ? -10000 : 0;	// room
																			// effect
																			// level
		normalEAX.lRoomHF = FlagAL_check(ch, AL_FLAGS_FLAT2D) ? -10000 : 0;	// room
																			// effect
																			// level
																			// at
																			// high
																			// frequencies
		normalEAX.flRoomRolloffFactor = 0;	// like DS3D flRolloffFactor
											// but for room effect
		normalEAX.lObstruction = 0;	// main obstruction control
									// (attenuation at high frequencies)
		normalEAX.flObstructionLFRatio = 0;	// obstruction low-frequency
											// level re. main control
		normalEAX.lOcclusion = 0;	// main occlusion control (attenuation
									// at high frequencies)
		normalEAX.flOcclusionLFRatio = 0;	// occlusion low-frequency
											// level re. main control
		normalEAX.flOcclusionRoomRatio = 0;	// occlusion room effect level
											// re. main control
		normalEAX.lOutsideVolumeHF = 0;	// outside sound cone level at
										// high frequencies
		normalEAX.flAirAbsorptionFactor = 0;	// multiplies
												// DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF
		// modifies the behavior of properties
		normalEAX.dwFlags = FlagAL_check(ch, AL_FLAGS_FLAT2D)
			? 0
			: EAXBUFFERFLAGS_DIRECTHFAUTO + EAXBUFFERFLAGS_ROOMAUTO +
			EAXBUFFERFLAGS_ROOMHFAUTO;

		alSource_EAX_All(sourceNum, &normalEAX);
#endif

		alSourcePlay(sourceNum);
	}
}

void S_fastsound_queue(vec3_t origin, int entnum, int entchannel,
					   ALuint bufferNum, float fvol, float attenuation,
					   unsigned timeofs)
{
	playsound_t *ps, *sort;

	if (!s_openal_numChannels || !bufferNum)
		return;

	// willow: Immediate sound effect start in case zero time delay
	// provided.
	// This is shortcut, just call "S_fastsound" call instead
	if (!timeofs) {
		S_fastsound(origin, entnum, entchannel, bufferNum, fvol,
					attenuation);
		return;
	}
	// Allocate a playSound
	ps = S_AllocPlaysound();
	if (!ps) {
		Com_Printf("S_StartSound: active-task queue fault\n");
		return;
	}

	ps->bufferNum = bufferNum;
	ps->entnum = entnum;
	ps->entchannel = entchannel;

//  ps->flat = is_flat;

	if (origin) {
		ps->fixed_origin = true;
		VectorCopy(origin, ps->origin);
	} else
		ps->fixed_origin = false;

	ps->volume = fvol;
	ps->attenuation = attenuation;
	ps->begin = cl.time + timeofs;

	// Sort into the pending playSounds list
	for (sort = s_pendingplays.next;
		 sort != &s_pendingplays && sort->begin < ps->begin;
		 sort = sort->next);

	ps->next = sort;
	ps->prev = sort->prev;

	ps->next->prev = ps;
	ps->prev->next = ps;
}

// picks a channel based on priorities, empty slots, number of channels
openal_channel_t *PickChannel_lite(ALuint * sourceNum)
{
	openal_channel_t *ch;
	int i;
	int firstToDie = -1;
	qboolean terminate = true;

	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		 i++, ch++) {
		// Check if this channel is active
/*		if (!ch->sfx)							// found free channel
		{
			firstToDie = i;
			terminate = false;
			break;
		} else {*/
		ALuint SourceState;

		alGetSourcei(source_name[i], AL_SOURCE_STATE, &SourceState);

		// The source already out of processing.
		if (SourceState == AL_STOPPED || SourceState == AL_INITIAL) {
			firstToDie = i;
			terminate = false;
			break;
		}
/*		}*/
	}

	// Emergency channel re-caption. Issue 2
	// Terminate the most distant entity
	if (firstToDie == -1) {
		// vec3_t listener;
		float len, max = 0;
		vec3_t delta;
		// alGetListenerfv(AL_POSITION, (ALfloat *)&listener);

		for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
			 i++, ch++)
			if (!FlagAL_check(ch, AL_FLAGS_FLAT2D)	// Don't touch FLAT 2D
													// samples!
				&& FlagAL_check(ch, AL_FLAGS_AL_LOOPING)
				) {
				// ch->_AL_POSITION must have same data:
				// alGetSourcefv(ch->sourceNum, AL_POSITION, (ALfloat
				// *)&al_position);
				// willow: i am assume cl.refdef.vieworg always have the
				// last listener position
				VectorSubtract(cl.refdef.vieworg, ch->_AL_POSITION, delta);
				// willow:
				// actually we need vector length here, but squared length
				// can do the same job faster.
				len = VectorLength_Squared(delta);
				if (len > max) {
					firstToDie = i;
					max = len;
				}
			}

		if (firstToDie == -1) {
			return NULL;
		}
	}

	ch = &s_openal_channels[firstToDie];

	memset(ch->_AL_POSITION, 0, sizeof(vec3_t));

	ch->entNum = -1;
	ch->entChannel = 0;
	// ch->startTime = cl.time;

	*sourceNum = source_name[firstToDie];

	// TERMINATE CHANNEL (stop working channel)
	if (terminate) {
		alSourceStop(*sourceNum);
	}

	return ch;
}

/*
==================
S_StartLocalSound
==================
*/
void S_StartLocalSound(ALuint bufferNum)
{
#ifdef _WITH_EAX
	EAXBUFFERPROPERTIES nullEAX;
#endif
	// long lDirect; // direct path level
	// long lDirectHF; // direct path level at high frequencies
	// long lRoom; // room effect level
	// long lRoomHF; // room effect level at high frequencies
	// float flRoomRolloffFactor; // like DS3D flRolloffFactor but for
	// room effect
	// long lObstruction; // main obstruction control (attenuation at high
	// frequencies)
	// float flObstructionLFRatio; // obstruction low-frequency level re.
	// main control
	// long lOcclusion; // main occlusion control (attenuation at high
	// frequencies)
	// float flOcclusionLFRatio; // occlusion low-frequency level re. main
	// control
	// float flOcclusionRoomRatio; // occlusion room effect level re. main
	// control
	// long lOutsideVolumeHF; // outside sound cone level at high
	// frequencies
	// float flAirAbsorptionFactor; // multiplies
	// DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF
	// unsigned long dwFlags; // modifies the behavior of properties

	if (s_openal_numChannels && bufferNum) {
		ALuint sourceNum;
		// Pick a channel and start the sound effect
		openal_channel_t *ch = PickChannel_lite(&sourceNum);

		if (ch) {
			FlagAL_clear(ch, AL_FLAGS_AL_LOOPING);
			ch->bufferNum = bufferNum;
			FlagAL_set(ch, AL_FLAGS_FLAT2D | AL_FLAGS_FIXED_POSITION);
			alSourcef(sourceNum, AL_PITCH, 1);
			alSource3f(sourceNum, AL_DIRECTION, 0, 0, 0);
			alSourcef(sourceNum, AL_REFERENCE_DISTANCE, 0);
			alSourcef(sourceNum, AL_ROLLOFF_FACTOR, 0);
			alSource3f(sourceNum, AL_POSITION, 0, 0, 0);
			alSource3f(sourceNum, AL_VELOCITY, 0, 0, 0);
			alSourcei(sourceNum, AL_BUFFER, bufferNum);
			alSourcei(sourceNum, AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcei(sourceNum, AL_LOOPING, AL_FALSE);
			alSourcef(sourceNum, AL_GAIN, 0.47);

#ifdef _WITH_EAX
			// alSource_EAX_Flags(sourceNum, 0); //willow: should be no
			// EAX effects!
			nullEAX.lDirect = 0;	// direct path level
			nullEAX.lDirectHF = 0;	// direct path level at high
									// frequencies
			nullEAX.lRoom = -10000;	// room effect level
			nullEAX.lRoomHF = -10000;	// room effect level at high
										// frequencies
			nullEAX.flRoomRolloffFactor = 0;	// like DS3D
												// flRolloffFactor but for
												// room effect
			nullEAX.lObstruction = 0;	// main obstruction control
										// (attenuation at high
										// frequencies)
			nullEAX.flObstructionLFRatio = 0;	// obstruction
												// low-frequency level re.
												// main control
			nullEAX.lOcclusion = 0;	// main occlusion control (attenuation
									// at high frequencies)
			nullEAX.flOcclusionLFRatio = 0;	// occlusion low-frequency
											// level re. main control
			nullEAX.flOcclusionRoomRatio = 0;	// occlusion room effect
												// level re. main control
			nullEAX.lOutsideVolumeHF = 0;	// outside sound cone level at
											// high frequencies
			nullEAX.flAirAbsorptionFactor = 0;	// multiplies
												// DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF
			nullEAX.dwFlags = 0;	// modifies the behavior of properties
			alSource_EAX_All(sourceNum, &nullEAX);	// willow: should be
													// no EAX effects!
#endif
			// alEAXSet(&DSPROPSETID_EAX_BufferProperties,
			// DSPROPERTY_EAXBUFFER_NONE,
			// sourceNum,
			// 0,
			// 0);

			alSourcePlay(sourceNum);
		} else {
			Com_Printf("S_StartLocalSound: Dropped sound\n");
		}
	}
}


/*
==================
S_StopAllSounds
==================
*/
void S_StopAllSounds(void)
{
	unsigned i;
	openal_channel_t *ch;

	if (!s_openal_numChannels)
		return;

	// clear all the playsounds
	memset(s_playsounds, 0, sizeof(s_playsounds));
	s_freeplays.next = s_freeplays.prev = &s_freeplays;
	s_pendingplays.next = s_pendingplays.prev = &s_pendingplays;

	for (i = 0; i < MAX_PLAYSOUNDS; i++) {
		s_playsounds[i].prev = &s_freeplays;
		s_playsounds[i].next = s_freeplays.next;
		s_playsounds[i].prev->next = &s_playsounds[i];
		s_playsounds[i].next->prev = &s_playsounds[i];
	}

	// Stop all the channels
	alSourceStopv(s_openal_numChannels, source_name);

	// Mark all the channels free
	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		 i++, ch++) {
		ch->bufferNum = AL_NONE;
		alSourcei(source_name[i], AL_BUFFER, AL_NONE);
	}


	// clear all the channels
	// memset(s_openal_channels, 0, sizeof(s_openal_channels));

	// Stop streaming channel
	S_StopBackgroundTrack();
}

void S_RawSetup(unsigned rate, unsigned width, unsigned channels)
{
	if (!s_openal_numChannels)
		return;
}

void S_RawSamples(unsigned samples, byte * data)
{
	if (!s_openal_numChannels)
		return;
}


openal_channel_t *PickChannel(channel_task_t * Channels_TODO,
							  unsigned int entNum, unsigned int entChannel,
							  vec3_t new_vec, int *return_index,
							  vec3_t listener)
{
	openal_channel_t *ch;
	int i;
	int firstToDie = -1;
	qboolean terminate = false;

	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		 i++, ch++) {
		// Don't let game sounds override streaming sounds
		/* if (!i && streaming) continue; */

		// Check if this channel is active
		if (!ch->bufferNum)		// (!ch->sfx)
		{
			// Free channel
			firstToDie = i;
			break;
		}
/*		if (Flag_checkAL (&Channels_TODO[i], AL_TASK_MANAGER__TERMINATE))
		{
			firstToDie = i;
			terminate = false;
			break;
		}*/
		else {
			ALuint SourceState;

			alGetSourcei(source_name[i], AL_SOURCE_STATE, &SourceState);

			if (SourceState == AL_STOPPED || SourceState == AL_INITIAL)	// The
																		// source
																		// already
																		// out
																		// of
																		// processing.
			{
				firstToDie = i;
				break;
			}
		}

	}

	// Emergency channel re-caption.
	// Override sound from same entity willow???????
	/* if (firstToDie == -1) { for (i = 0, ch = s_openal_channels; i <
	   s_openal_numChannels; i++, ch++) if (ch->entNum == entNum &&
	   ch->entChannel == entChannel) { firstToDie = i; break; } } */

	// Emergency channel re-caption. Issue 2
	// Terminate the most distant entity
	if (firstToDie == -1) {
		float len, max;
		vec3_t delta;

		terminate = true;		// we need to eat weakest now :)
		if (new_vec) {
			VectorSubtract(listener, new_vec, delta);
			max = VectorLength_Squared(delta);
		} else
			max = 0;

		for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
			 i++, ch++)
			if (!FlagAL_check(ch, AL_FLAGS_FLAT2D)	// Don't touch FLAT 2D
													// samples!
				&& FlagAL_check(ch, AL_FLAGS_AL_LOOPING)
				) {
				// ch->_AL_POSITION must have same data:
				// alGetSourcefv(ch->sourceNum, AL_POSITION, (ALfloat
				// *)&al_position);
				VectorSubtract(listener, ch->_AL_POSITION, delta);
				// willow:
				// actually we need vector length here, but squared length
				// can do the same job faster.
				len = VectorLength_Squared(delta);
				if (len > max) {
					firstToDie = i;
					max = len;
				}
			}

		if (firstToDie == -1) {
			return NULL;
		}
	}

	ch = &s_openal_channels[firstToDie];

	if (new_vec) {
		memcpy(ch->_AL_POSITION, new_vec, sizeof(vec3_t));
		FlagAL_clear(ch, AL_FLAGS_FLAT2D);
//      Flag_clear (&Channels_TODO[firstToDie],AL_TASK_MANAGER__IS_SOURCE_RELATIVE);
	} else {
		memset(ch->_AL_POSITION, 0, sizeof(vec3_t));
		FlagAL_set(ch, AL_FLAGS_FLAT2D);
//      Flag_set (&Channels_TODO[firstToDie],AL_TASK_MANAGER__IS_SOURCE_RELATIVE);
	}

	ch->entNum = entNum;
	ch->entChannel = entChannel;
	// ch->startTime = cl.time;
	if (return_index)
		*return_index = firstToDie;

	// TERMINATE CHANNEL (stop working channel)
	if (terminate)
		TASK_TerminateChannel(&Channels_TODO[firstToDie], ch);
	// alSourceStop(ch->sourceNum);
	// alSourcei(ch->sourceNum, AL_BUFFER, 0);

	return ch;
}

/*
============
S_Update

Called once each time through the main loop
============
*/
void S_Update(vec3_t listener_position, vec3_t velocity,
			  float orientation[6])
{
	entity_state_t *ent;
	int i, j;
	byte cl_parse_entities_goodjob[MAX_PARSE_ENTITIES];
	playsound_t *ps;
	openal_channel_t *ch;
	channel_task_t Channels_TODO[MAX_CHANNELS];
	int logical_channel_index;

	if (!s_openal_numChannels)
		return;
	
	// Set up listener
	alListenerfv(AL_POSITION, listener_position);
	alListenerfv(AL_VELOCITY, velocity);
	alListenerfv(AL_ORIENTATION, orientation);

	if((CL_PMpointcontents(listener_position) & MASK_WATER))
		alSpeedOfSound(59000);
	else
		alSpeedOfSound(13515);


#ifdef _HAVE_EAX
//willow: mattn sure we have EAX on win only, so do i
	// If EAX is enabled, apply listener environmental effects
	if (alConfig.eax >= 2 && alConfig.eax <= 5) {
		unsigned long ulEAXValLF = -150;  //range form 0 to -10000
		unsigned long ulEAXValHF = 0;
		unsigned long eaxEnv;			//reverberation type
		unsigned long eaxEnvSize = 75; // surround room size def 7.5 max 100
		unsigned long eaxReverb = 0; //reverberation level min - 10000 max 2000
		unsigned long eaxReverbDelay = 0.1;  //reverberation delay (sec) range 0.0 to 0.1
		
		if (cls.state != ca_active) {
			eaxEnv = EAX_ENVIRONMENT_GENERIC;
		} else {
			eaxEnv = (CL_PMpointcontents(listener_position) & MASK_WATER)
				? EAX_ENVIRONMENT_UNDERWATER : EAX_ENVIRONMENT_HANGAR;
		}

		if (eaxEnv != alConfig.eaxState) {
			if (eaxEnv == EAX_ENVIRONMENT_GENERIC)
				Com_DPrintf("EAXSet: EAX_ENVIRONMENT_GENERIC\n");
			else
				Com_DPrintf("EAXSet: EAX_ENVIRONMENT_UNDERWATER\n");

			alConfig.eaxState = eaxEnv;

//EAXSet description:
//This function sets an EAX value.
//C Specification:
//ALenum EAXSet(const struct _GUID *propertySetID,ALuint property,ALuint source,ALvoid
//*value,ALuint size);
//Parameters:
//*propertySetID - A pointer to the property set GUID of the object being set (a listener or a source)
//property - The property being set
//source - The ID of the source to be set
//*value - A pointer to the value being returned
//size - The size of the data storage area pointed to by *value
//Return Value: An OpenAL error code indicating if there was an error in setting the data
			alEAXSet(&DSPROPSETID_EAX_ListenerProperties,
					 DSPROPERTY_EAXLISTENER_ROOM, 0, &ulEAXValLF,
					 sizeof(unsigned long));

			alEAXSet(&DSPROPSETID_EAX_ListenerProperties,
					 DSPROPERTY_EAXLISTENER_ROOMHF, 0, &ulEAXValHF,
					 sizeof(unsigned long));

			alEAXSet(&DSPROPSETID_EAX_ListenerProperties,
					 DSPROPERTY_EAXLISTENER_ENVIRONMENT, 0, &eaxEnv,
					 sizeof(unsigned long));
			
			alEAXSet(&DSPROPSETID_EAX_ListenerProperties,
					 DSPROPERTY_EAXLISTENER_ENVIRONMENTSIZE, 0, &eaxEnvSize,
					 sizeof(unsigned long));

			alEAXSet(&DSPROPSETID_EAX_ListenerProperties,
					 DSPROPERTY_EAXLISTENER_REVERB, 0, &eaxReverb,
					 sizeof(unsigned long));

			alEAXSet(&DSPROPSETID_EAX_ListenerProperties,
					 DSPROPERTY_EAXLISTENER_REVERBDELAY, 0, &eaxReverbDelay,
					 sizeof(unsigned long));
			

		}


	}
#endif

	memset(Channels_TODO, 0, sizeof(Channels_TODO));

	// Add looping sounds
	if (!(cls.state != ca_active || cl_paused->value)) {
		memset(cl_parse_entities_goodjob, 0, MAX_PARSE_ENTITIES);

		for (i = cl.frame.parse_entities;
			 i < cl.frame.num_entities + cl.frame.parse_entities; i++) {
			int idx = i & (MAX_PARSE_ENTITIES - 1);
			ALuint sfx_numm;
			ent = &cl_parse_entities[idx];
			if (!ent->sound) {
				cl_parse_entities_goodjob[idx] = 1;
				continue;		// No sound effect
			}

			sfx_numm = cl.sound_precache[ent->sound];
			if (!sfx_numm) {
				cl_parse_entities_goodjob[idx] = 1;
				continue;		// Bad sound effect
			}
			// If this entity is already playing the same sound effect on
			// an
			// active channel, then simply update it
			for (j = 0, ch = s_openal_channels; j < s_openal_numChannels;
				 j++, ch++) {
//              if (ch->sfx != sfx) continue;
				if (ch->bufferNum != sfx_numm)
					continue;

				if (ch->entNum != ent->number)
					continue;

				if (!FlagAL_check(ch, AL_FLAGS_AL_LOOPING))
					continue;

				Flag_set(&Channels_TODO[j],
						 AL_TASK_MANAGER__IS_LOOP_ACTIVE);

				{
					vec3_t position;
					vec3_t delta;
					CL_GetEntityOrigin(ch->entNum, position);

					VectorSubtract(position, ch->_AL_POSITION, delta);
					VectorScale(delta, 1 / 30, delta);

					alSourcefv(source_name[j], AL_VELOCITY, delta);
					alSourcefv(source_name[j], AL_POSITION, position);
					// memcpy(Channels_TODO[j].TASK_AL_VELOCITY, delta,
					// sizeof(vec3_t));
					memcpy(ch->_AL_POSITION, position, sizeof(vec3_t));
				}

				cl_parse_entities_goodjob[idx] = 1;

				break;			// keep channel alive.
			}
		}
	}
	// Check for stop, no-loops spatialization
	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		 i++, ch++) {
		if (ch->bufferNum)		// (ch->sfx)
		{
			ALuint SourceState;

			alGetSourcei(source_name[i], AL_SOURCE_STATE, &SourceState);

			if (SourceState == AL_STOPPED || SourceState == AL_INITIAL)	// The
																		// source
																		// already
																		// out
																		// of
																		// processing.
			{
				ch->bufferNum = 0;
				// alSourcei(ch->sourceNum, AL_BUFFER, 0);
			} else {
				// Kill some loops
				if (FlagAL_check(ch, AL_FLAGS_AL_LOOPING)) {
					if (!Flag_check
						(&Channels_TODO[i],
						 AL_TASK_MANAGER__IS_LOOP_ACTIVE)) {
						TASK_TerminateChannel(&Channels_TODO[i], ch);
					}
				} else if (!FlagAL_check(ch, AL_FLAGS_FIXED_POSITION)) {
					vec3_t velocity, position;

					CL_GetEntityOrigin(ch->entNum, position);
					VectorSubtract(position, ch->_AL_POSITION, velocity);
					VectorScale(velocity, 1 / 30, velocity);

					alSourcefv(source_name[i], AL_VELOCITY, velocity);
					alSourcefv(source_name[i], AL_POSITION, position);
					memcpy(ch->_AL_POSITION, position, sizeof(vec3_t));
					// memcpy(Channels_TODO[i].TASK_AL_VELOCITY, velocity,
					// sizeof(vec3_t));
					// alSourcefv(ch->sourceNum, AL_DIRECTION, direction);
				}
			}
		}
	}

	if (!(cls.state != ca_active || cl_paused->value))
		for (i = cl.frame.parse_entities;
			 i < cl.frame.num_entities + cl.frame.parse_entities; i++) {
			if (!cl_parse_entities_goodjob[i & (MAX_PARSE_ENTITIES - 1)]) {
				vec3_t position;

				ent = &cl_parse_entities[i & (MAX_PARSE_ENTITIES - 1)];

				// Pick a channel and start the sound effect
				CL_GetEntityOrigin(ent->number, position);
			
			
				//          ????? ps->entnum, ps->entchannel ?????
				if ((ch = PickChannel(Channels_TODO, ent->number, 0, position,
						&logical_channel_index, listener_position))) {
					channel_task_t *current_task = &Channels_TODO[logical_channel_index];

					ch->bufferNum = cl.sound_precache[ent->sound];
					ch->entNum = ent->number;	// loopNum

					// ch->loopSound = AL_TRUE;
					FlagAL_set(ch, AL_FLAGS_AL_LOOPING);
					FlagAL_clear(ch, AL_FLAGS_FIXED_POSITION);

					// ch->distanceMult = 0.3f;
					// Update min/max distance
					current_task->TASK_AL_REFERENCE_DISTANCE = 27;	// willow:
																	// Player
																	// unit
																	// width
																	// * 3
																	// /
																	// 4;
					// Set max distance really far away (default)
					// alSourcef(ch->sourceNum, AL_MAX_DISTANCE, 65536);

					// Update volume and rolloff factor from hacking
					// database, the only decent source :(
					current_task->TASK_AL_ROLLOFF_FACTOR =
						cl.sound_precache_rolloff_factor[ent->sound];
					current_task->TASK_AL_GAIN =
						cl.sound_precache_gain[ent->sound];

//              alSourcefv(ch->sourceNum, AL_DIRECTION, ent->angles);
					{
						vec3_t delta, delta2;
						VectorSubtract(position, ent->old_origin, delta2);
						VectorScale(delta2, 1 / 30, delta);
						memcpy(current_task->TASK_AL_VELOCITY, delta,
							   sizeof(vec3_t));
						// memcpy(current_task->TASK_AL_VELOCITY,
						// ent->delta, sizeof(vec3_t));
					}

//              current_task->TASK_AL_SOURCE_RELATIVE = AL_FALSE;
					Flag_set(current_task, AL_TASK_MANAGER__EXECUTE);
				}
			}
		}
	// Issue playSounds
	for (;;) {
		ps = s_pendingplays.next;

		if (ps == &s_pendingplays)
			break;				// No more pending playSounds

		if (ps->begin > cl.time)
			break;				// No more pending playSounds this frame

		if (ps->fixed_origin)
			ch = PickChannel(Channels_TODO, ps->entnum, ps->entchannel,
							 ps->origin, &logical_channel_index,
							 listener_position);
		else {
			vec3_t position;
			CL_GetEntityOrigin(ps->entnum, position);
			ch = PickChannel(Channels_TODO, ps->entnum, ps->entchannel,
							 position, &logical_channel_index,
							 listener_position);
		}


		if (ch) {
			channel_task_t *current_task =
				&Channels_TODO[logical_channel_index];

			// VectorCopy(ps->origin, ch->position);
			ch->bufferNum = ps->bufferNum;	// ch->sfx = ps->sfx;


			{
				// Update min/max distance
				current_task->TASK_AL_REFERENCE_DISTANCE = 28;	// 34;
				// Set max distance really far away (default)
				// alSourcef(ch->sourceNum, AL_MAX_DISTANCE, 65536);

				current_task->TASK_AL_ROLLOFF_FACTOR = ps->attenuation;
				current_task->TASK_AL_GAIN = ps->volume;

				// SpatializeChannel(ch, ps->origin);
				// memcpy(ch->_AL_POSITION, ps->origin, sizeof(vec3_t));
				// //alSourcefv(ch->sourceNum, AL_POSITION, ps->origin);
				// //ch->position);
				// alSource3f(ch->sourceNum, AL_VELOCITY, 0, 0, 0);

				if (ps->fixed_origin) {
					FlagAL_clear(ch, AL_FLAGS_AL_LOOPING);
					FlagAL_set(ch, AL_FLAGS_FIXED_POSITION);
					memset(current_task->TASK_AL_VELOCITY, 0,
						   sizeof(vec3_t));
				} else {
					FlagAL_clear(ch,
								 AL_FLAGS_FIXED_POSITION |
								 AL_FLAGS_AL_LOOPING);
					// willow: TO DO.
					memset(current_task->TASK_AL_VELOCITY, 0,
						   sizeof(vec3_t));
				}

//              current_task->TASK_AL_SOURCE_RELATIVE = AL_FALSE;
			}
			Flag_set(current_task, AL_TASK_MANAGER__EXECUTE);
		}
		// Free the playSound
		S_FreePlaysound(ps);
	}

	// Direct access to OpenAL layer
	for (i = 0, ch = s_openal_channels; i < s_openal_numChannels;
		 i++, ch++) {
		channel_task_t *current_task = &Channels_TODO[i];

		if (Flag_check(current_task, AL_TASK_MANAGER__TERMINATE)) {
			alSourceStop(source_name[i]);
		}

		if (Flag_check(current_task, AL_TASK_MANAGER__EXECUTE)) {
#ifdef _WITH_EAX
			EAXBUFFERPROPERTIES normalEAX;
#endif

			ALuint sourceNum = source_name[i];
			alSourcef(sourceNum, AL_PITCH, 1);
			alSource3f(sourceNum, AL_DIRECTION, 0, 0, 0);
			alSourcef(sourceNum, AL_REFERENCE_DISTANCE,
					  current_task->TASK_AL_REFERENCE_DISTANCE);
			alSourcef(sourceNum, AL_ROLLOFF_FACTOR,
					  current_task->TASK_AL_ROLLOFF_FACTOR);
			alSourcefv(sourceNum, AL_POSITION, ch->_AL_POSITION);
			alSourcefv(sourceNum, AL_VELOCITY,
					   current_task->TASK_AL_VELOCITY);
			alSourcei(sourceNum, AL_BUFFER, ch->bufferNum);
			alSourcei(sourceNum, AL_SOURCE_RELATIVE, FlagAL_checkAL(ch, AL_FLAGS_FLAT2D));	// Flag_checkAL
																							// (current_task,
																							// AL_TASK_MANAGER__IS_SOURCE_RELATIVE));
			alSourcei(sourceNum, AL_LOOPING, FlagAL_checkAL(ch, AL_FLAGS_AL_LOOPING));	// ch->loopSound);
			alSourcef(sourceNum, AL_GAIN, current_task->TASK_AL_GAIN);

#ifdef _WITH_EAX
		normalEAX.lDirect = 0;	// direct path level
			normalEAX.lDirectHF = 0;	// direct path level at high
										// frequencies
			normalEAX.lRoom = FlagAL_check(ch, AL_FLAGS_FLAT2D) ? -10000 : 0;	// room
																				// effect
																				// level
			normalEAX.lRoomHF = FlagAL_check(ch, AL_FLAGS_FLAT2D) ? -10000 : 0;	// room
																				// effect
																				// level
																				// at
																				// high
																				// frequencies
			normalEAX.flRoomRolloffFactor = 0;	// like DS3D
												// flRolloffFactor but for
												// room effect
			normalEAX.lObstruction = 0;	// main obstruction control
										// (attenuation at high
										// frequencies)
			normalEAX.flObstructionLFRatio = 0;	// obstruction
												// low-frequency level re.
												// main control
			normalEAX.lOcclusion = 0;	// main occlusion control
										// (attenuation at high
										// frequencies)
			normalEAX.flOcclusionLFRatio = 0;	// occlusion low-frequency
												// level re. main control
			normalEAX.flOcclusionRoomRatio = 0;	// occlusion room effect
												// level re. main control
			normalEAX.lOutsideVolumeHF = 0;	// outside sound cone level at
											// high frequencies
			normalEAX.flAirAbsorptionFactor = 0;	// multiplies
													// DSPROPERTY_EAXLISTENER_AIRABSORPTIONHF
			// modifies the behavior of properties
			normalEAX.dwFlags = FlagAL_check(ch, AL_FLAGS_FLAT2D)
				? 0
				: EAXBUFFERFLAGS_DIRECTHFAUTO + EAXBUFFERFLAGS_ROOMAUTO +
				EAXBUFFERFLAGS_ROOMHFAUTO;

			alSource_EAX_All(sourceNum, &normalEAX);
#endif /* __WITH_EAX */

			alSourcePlay(sourceNum);
		}
	}

	S_UpdateBackgroundTrack();
}


/*
===============================================================================

console functions

===============================================================================
*/

void S_Play(void)
{
	int i = 1;
	char name[256];

	while (i < Cmd_Argc()) {
		if (!strrchr(Cmd_Argv(i), '.')) {
			strcpy(name, Cmd_Argv(i));
			strcat(name, ".wav");
		} else
			strcpy(name, Cmd_Argv(i));

		// TO DO - willow: do not cache this data to onboard memory!
		// this seems to be just any random file, we do no need to store
		// it in valuable memory.
		S_StartLocalSound(S_FindName(name, true));
		i++;
	}
}

void S_SoundList_f(void)
{
}


int cinframe;


void S_StopBackgroundTrack()
{
	if (streaming) {
		// Stop the Source and clear the Queue
		alSourceStop(source_name[s_openal_numChannels]);
		alSourcei(source_name[s_openal_numChannels], AL_BUFFER, 0);

		// Close Wave Handle
		if (streaming == 1)		// music streaming
		{
			StreamingWav_close();
		}

		streaming = 0;
	}
}

void S_UpdateBackgroundTrack()
{
	if (streaming) {
		ALint iState;
		ALenum format;
		int iBuffersProcessed = 0;

		if (cinframe < 4 && streaming == 2) {
			void *cin_data;
			SCR_audioCinematic(&cin_data, &stream_info_rate,
							   &ulBytesWritten, &format);
			if (ulBytesWritten) {
				alBufferData(uiBuffers[cinframe], format, cin_data,
							 ulBytesWritten, stream_info_rate);
				alSourceQueueBuffers(source_name[s_openal_numChannels], 1,
									 &uiBuffers[cinframe]);
				cinframe++;
			}
		} else
			// Request the number of OpenAL Buffers have been processed
			// (played) on the Source
			alGetSourcei(source_name[s_openal_numChannels],
						 AL_BUFFERS_PROCESSED, &iBuffersProcessed);

		// For each processed buffer, remove it from the Source Queue,
		// read next chunk of audio
		// data from disk, fill buffer with new data, and add it to the
		// Source Queue
		while (iBuffersProcessed) {
			// Remove the Buffer from the Queue.  (uiBuffer contains the
			// Buffer ID for the unqueued Buffer)
			ALuint uiBuffer = 0;
			alSourceUnqueueBuffers(source_name[s_openal_numChannels], 1,
								   &uiBuffer);

			  if (streaming == 1)     // music streaming
               {
				a:
                    ulBytesWritten =
                         (ulBufferSize >
                          stream_info_samples * 2) ? stream_info_samples *
                         2 : ulBufferSize;
                    if (ulBytesWritten) {
                         memcpy(pData, stream_wav + stream_info_dataofs,
                                 ulBytesWritten);
                         {
                              alBufferData(uiBuffer, AL_FORMAT_STEREO16, pData,
                                              ulBytesWritten, stream_info_rate);
                              alSourceQueueBuffers(source_name
                                                        [s_openal_numChannels], 1,
                                                        &uiBuffer);
                         }
                         stream_info_samples -= ulBytesWritten / 2;
                         stream_info_dataofs += ulBytesWritten;
                    }
                    else
                    {
                         if (BackgroundTrack_Repeat)
                              StreamingWav_Reset();
                         goto a;
                    }
               }
			if (streaming == 2)	// cinematic streaming
			{
				void *cin_data;
				SCR_audioCinematic(&cin_data, &stream_info_rate,
								   &ulBytesWritten, &format);
				if (ulBytesWritten) {
					{
						alBufferData(uiBuffer, format, cin_data,
									 ulBytesWritten, stream_info_rate);
						alSourceQueueBuffers(source_name
											 [s_openal_numChannels], 1,
											 &uiBuffer);
					}
				}
			}

			iBuffersProcessed--;
		}

		// Check the status of the Source.  If it is not playing, then
		// playback was completed,
		// or the Source was starved of audio data, and needs to be
		// restarted.

		if (cinframe < 4 && streaming == 2)
			return;

		alGetSourcei(source_name[s_openal_numChannels], AL_SOURCE_STATE,
					 &iState);
		  if (iState != AL_PLAYING) {
               ALint iQueuedBuffers;
               // If there are Buffers in the Source Queue then the Source
               // was starved of audio
               // data, so needs to be restarted (because there is more audio
               // data to play)
               alGetSourcei(source_name[s_openal_numChannels],
                               AL_BUFFERS_QUEUED, &iQueuedBuffers);
               if (iQueuedBuffers) {
                    alSourcePlay(source_name[s_openal_numChannels]);
               } else {
                    if (BackgroundTrack_Repeat)
                    // Repeat track
                         StreamingWav_Reset();
                    else
                    // Finished playing
                         S_StopBackgroundTrack();
               }
		}
	}
}


/*
 =================
 S_Music_f
 =================
*/


void S_Music_f(void)
{

	char intro[MAX_QPATH], loop[MAX_QPATH];

	Com_Printf("*** MUSIC BOX HIGHLY BETA!! ***\n");

	if (Cmd_Argc() < 2 || Cmd_Argc() > 3) {
		Com_Printf("Usage: music <musicfile> [loopfile]\n");
		return;
	}

	Q_strncpyz(intro, Cmd_Argv(1), sizeof(intro));
	Com_DefaultPath(intro, sizeof(intro), "music");
	Com_DefaultExtension(intro, sizeof(intro), ".wav");

	if (Cmd_Argc() == 3) {
		sprintf(loop, Cmd_Argv(2), sizeof(loop));
		Com_DefaultPath(loop, sizeof(loop), "music");
		Com_DefaultExtension(loop, sizeof(loop), ".wav");

		S_StartBackgroundTrack(intro, loop);
	} else
		S_StartBackgroundTrack(intro, intro);
}




void S_Play_Wav_Music(void)
{
	char	name[MAX_QPATH];
	int		track;
	
	if(cd_nocd->value){
		CDAudio_Stop();
		S_StopBackgroundTrack();	
		return;
	}
	track = atoi(cl.configstrings[CS_CDTRACK]);

	if (track == 0){
		// Stop any playing track
		CDAudio_Stop();
		S_StopBackgroundTrack();
		return;
	}

	// If an wav file exists play it, otherwise fall back to CD audio
	Q_snprintfz(name, sizeof(name), "music/track%02i.wav", track);
	if (FS_LoadFile(name, NULL) != -1)
		S_StartBackgroundTrack(name, name);
	else
		CDAudio_Play(track, true);

}

void S_StartBackgroundTrack(char *introTrack, char *loopTrack)
{
	 BackgroundTrack_Repeat = loopTrack != 0;
	
	if (streaming)
		S_StopBackgroundTrack();

	if (StreamingWav_init(loopTrack ? loopTrack : introTrack)) {
		int i;
		// Fill all the Buffers with audio data from the wavefile
		for (i = 0; i < SND_STREAMING_NUMBUFFERS; i++) {
/*				if (SUCCEEDED(pWaveLoader->ReadWaveData(WaveID, pData, ulBufferSize, &ulBytesWritten)))
*/
			ulBytesWritten =
				(ulBufferSize >
				 stream_info_samples * 2) ? stream_info_samples *
				2 : ulBufferSize;
			if (ulBytesWritten) {
				memcpy(pData, stream_wav + stream_info_dataofs,
					   ulBytesWritten);
				{
					alBufferData(uiBuffers[i], AL_FORMAT_STEREO16, pData,
								 ulBytesWritten, stream_info_rate);
					alSourceQueueBuffers(source_name[s_openal_numChannels],
										 1, &uiBuffers[i]);
				}
				stream_info_samples -= ulBytesWritten / 2;
				stream_info_dataofs += ulBytesWritten;
			}
		}
		// Start playing source
		alSourcef(source_name[s_openal_numChannels], AL_GAIN,
				  s_musicvolume->value);
		alSourcePlay(source_name[s_openal_numChannels]);
		streaming = 1;

	} else
		Com_Printf("Failed to load %s\n", introTrack);
}

void S_StartCinematic(void)
{
	if(!s_initsound->value || openalStop)
		return;
	
	if (streaming)
		S_StopBackgroundTrack();

	alSourcef(source_name[s_openal_numChannels], AL_GAIN, 1.0);

	cinframe = 0;
	streaming = 2;				// Streaming CINEMATICS audio
}
