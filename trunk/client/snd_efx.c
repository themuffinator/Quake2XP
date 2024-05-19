/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

#include "client.h"
#include "snd_loc.h"
#include "AL/efx-presets.h"

EFXEAXREVERBPROPERTIES rvb_generic			= EFX_REVERB_PRESET_GENERIC;
EFXEAXREVERBPROPERTIES rvb_underwater		= EFX_REVERB_PRESET_UNDERWATER;

EFXEAXREVERBPROPERTIES rvb_alcove			= EFX_REVERB_PRESET_FACTORY_ALCOVE;
EFXEAXREVERBPROPERTIES rvb_small_room		= EFX_REVERB_PRESET_FACTORY_SMALLROOM;
EFXEAXREVERBPROPERTIES rvb_medium_room		= EFX_REVERB_PRESET_FACTORY_MEDIUMROOM;
EFXEAXREVERBPROPERTIES rvb_large_room		= EFX_REVERB_PRESET_FACTORY_LARGEROOM;
EFXEAXREVERBPROPERTIES rvb_hall				= EFX_REVERB_PRESET_FACTORY_HALL;
EFXEAXREVERBPROPERTIES rvb_courtYard		= EFX_REVERB_PRESET_FACTORY_COURTYARD;

extern cvar_t *s_dynamicReverberation;

typedef struct {
	bool on;
	ALuint rvbGenericEffect;
	ALuint rvbUnderwaterEffect;
	ALuint rvbAuxSlot;

	ALuint rvbSmallRoomEffect;
	ALuint rvbMediumRoomEffect;
	ALuint rvbLargeRoomEffect;
	ALuint rvbFactoryHall;
	ALuint rvbFactoryCourtYard;
	ALuint rvbFactoryAlcove;
} efx_t;

efx_t efx;

ALuint EFX_RvbCreate(EFXEAXREVERBPROPERTIES *rvb, const char *name) {
	ALuint effect = 0;
	ALenum err;

	alGenEffects(1, &effect);
	
	Com_Printf("Load " S_COLOR_YELLOW "%s" S_COLOR_WHITE " effect: ", name);

	alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);

	alEffectf(effect, AL_EAXREVERB_DENSITY, rvb->flDensity);
	alEffectf(effect, AL_EAXREVERB_DIFFUSION, rvb->flDiffusion);
	alEffectf(effect, AL_EAXREVERB_GAIN, rvb->flGain);
	alEffectf(effect, AL_EAXREVERB_GAINHF, rvb->flGainHF);
	alEffectf(effect, AL_EAXREVERB_GAINLF, rvb->flGainLF);
	alEffectf(effect, AL_EAXREVERB_DECAY_TIME, rvb->flDecayTime);
	alEffectf(effect, AL_EAXREVERB_DECAY_HFRATIO, rvb->flDecayHFRatio);
	alEffectf(effect, AL_EAXREVERB_DECAY_LFRATIO, rvb->flDecayLFRatio);
	alEffectf(effect, AL_EAXREVERB_REFLECTIONS_GAIN, rvb->flReflectionsGain);
	alEffectf(effect, AL_EAXREVERB_REFLECTIONS_DELAY, rvb->flReflectionsDelay);
	alEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN, rvb->flReflectionsPan);
	alEffectf(effect, AL_EAXREVERB_LATE_REVERB_GAIN, rvb->flLateReverbGain);
	alEffectf(effect, AL_EAXREVERB_LATE_REVERB_DELAY, rvb->flLateReverbDelay);
	alEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN, rvb->flLateReverbPan);
	alEffectf(effect, AL_EAXREVERB_ECHO_TIME, rvb->flEchoTime);
	alEffectf(effect, AL_EAXREVERB_ECHO_DEPTH, rvb->flEchoDepth);
	alEffectf(effect, AL_EAXREVERB_MODULATION_TIME, rvb->flModulationTime);
	alEffectf(effect, AL_EAXREVERB_MODULATION_DEPTH, rvb->flModulationDepth);
	alEffectf(effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, rvb->flAirAbsorptionGainHF);
	alEffectf(effect, AL_EAXREVERB_HFREFERENCE, rvb->flHFReference);
	alEffectf(effect, AL_EAXREVERB_LFREFERENCE, rvb->flLFReference);
	alEffectf(effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, rvb->flRoomRolloffFactor);
	alEffecti(effect, AL_EAXREVERB_DECAY_HFLIMIT, rvb->iDecayHFLimit);

	err = alGetError();
	if (err != AL_NO_ERROR)
	{
		Com_Printf("EFX_RvbCreate error: %s\n", alGetString(err));
		if (alIsEffect(effect))
			alDeleteEffects(1, &effect);
		return 0;
	}
	
	Com_Printf(S_COLOR_GREEN"ok\n");

	return effect;
}


void EFX_RvbInit (void) {

	if (efx.on)
		return;

	Com_Printf("=====================================\n");
	ALint major, minor;
	alcGetIntegerv(alConfig.hDevice, ALC_EFX_MAJOR_VERSION, 1, &major);
	alcGetIntegerv(alConfig.hDevice, ALC_EFX_MINOR_VERSION, 1, &minor);
	
	Com_Printf("\n");
	Com_Printf("EFX_VERSION: " S_COLOR_GREEN "%i.%i\n\n", major, minor);
	Com_Printf("Load EFX Presets...\n\n");

	efx.rvbGenericEffect		= EFX_RvbCreate (&rvb_generic, "GENERIC");
	efx.rvbUnderwaterEffect		= EFX_RvbCreate (&rvb_underwater, "UNDERWATHER");

	efx.rvbSmallRoomEffect		= EFX_RvbCreate(&rvb_small_room, "SMALL ROOM");
	efx.rvbMediumRoomEffect		= EFX_RvbCreate(&rvb_medium_room, "MEDIUM ROOM");
	efx.rvbLargeRoomEffect		= EFX_RvbCreate(&rvb_large_room, "LARGE ROOM");
	efx.rvbFactoryHall			= EFX_RvbCreate(&rvb_hall, "HALL");
	efx.rvbFactoryCourtYard		= EFX_RvbCreate(&rvb_courtYard, "COURTYARD");
	efx.rvbFactoryAlcove		= EFX_RvbCreate(&rvb_alcove, "ALCOVE");

	alGenAuxiliaryEffectSlots(1, &efx.rvbAuxSlot);
	alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_AUXILIARY_SEND_AUTO, AL_TRUE);

	if (alGetError () == AL_NO_ERROR) {
		efx.on = true;
	}
	else {
		Com_Printf (S_COLOR_RED "failed!\n");
		efx.on = false;
	}
	Com_Printf("\n");
}

void EFX_GetRoomSize() {
	vec3_t len;
	trace_t trace;
	int avr = 0;

	if (CL_PMpointcontents(cl.refdef.vieworg) & CONTENTS_SOLID)
		return;

	if (!cl.refresh_prepped)
		return;

	if (!s_dynamicReverberation->integer) {
		alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbGenericEffect);
		return;
	}

	vec3_t dir[6] = {
	{8192, 0, 0 },	// forward 
	{-8192, 0, 0},	// back
	{0, 8192, 0 },	// left 
	{0, -8192, 0},	// right
	{0, 0, 8192 },	// up 
	{0, 0, -8192},	// down
	};

	for (int i = 0; i < 6; i++){

		trace = CL_PMTraceWorld(cl.refdef.vieworg, vec3_origin, vec3_origin, dir[i], MASK_SOLID, false);
		if (trace.fraction > 0 && trace.fraction < 1) {
			VectorSubtract(trace.endpos, cl.refdef.vieworg, len);
			avr += VectorLength(len);
		}
	}
	avr /= 5;

//	Com_Printf("%i\n", avr);

	if (avr < 128)
		alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbFactoryAlcove);

	if (avr >= 128 && avr < 250)
		alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbSmallRoomEffect);
	
	if (avr >= 250 && avr < 350)
		alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbMediumRoomEffect);

	if (avr >= 350 && avr < 450)
		alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbLargeRoomEffect);
	
	if (avr >= 450)
		alAuxiliaryEffectSloti(efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbFactoryCourtYard);
}


char* al_error(int err)
{
	if (err == AL_INVALID_NAME)
		return "AL_INVALID_NAME";
	else if (err == AL_INVALID_ENUM)
		return "AL_INVALID_ENUM";
	else if (err == AL_INVALID_VALUE)
		return "AL_INVALID_VALUE";
	else if (err == AL_INVALID_OPERATION)
		return "AL_INVALID_OPERATION";
	else if (err == AL_OUT_OF_MEMORY)
		return "AL_OUT_OF_MEMORY";
	else if (err == AL_NO_ERROR)
		return "AL_NO_ERROR";
	return va("0x%X", err);
}

void EFX_RvbUpdate (vec3_t listener_position) {
	
	if (!efx.on)
		return;
	
	if (!cl.refresh_prepped)
		return;

	// If we are not playing, use default preset
	if (cls.state != ca_active) {
		alAuxiliaryEffectSloti (efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbGenericEffect);
	}
	else if (CL_PMpointcontents (listener_position) & MASK_WATER) {
		// Check if we are underwater and update data
		alAuxiliaryEffectSloti (efx.rvbAuxSlot, AL_EFFECTSLOT_EFFECT, efx.rvbUnderwaterEffect);
	}
	else {
		EFX_GetRoomSize();
	}

/*	unsigned err; // errors with creative cards
	err = alGetError();
	if (err != AL_NO_ERROR)
		Com_Printf (S_COLOR_RED "EFX update failed, error %s\n", al_error(err));
*/		
}

void EFX_RvbShutdown (void) {
	if (!efx.on)
		return;

	Com_Printf ("EFX shutdown\n");
	alDeleteAuxiliaryEffectSlots (1, &efx.rvbAuxSlot);
	alDeleteEffects (1, &efx.rvbGenericEffect);
	alDeleteEffects (1, &efx.rvbUnderwaterEffect);

	alDeleteEffects(1, &efx.rvbLargeRoomEffect);
	alDeleteEffects(1, &efx.rvbMediumRoomEffect);
	alDeleteEffects(1, &efx.rvbSmallRoomEffect);
	alDeleteEffects(1, &efx.rvbFactoryCourtYard);
	alDeleteEffects(1, &efx.rvbFactoryHall);
	alDeleteEffects(1, &efx.rvbFactoryAlcove);

	efx.on = false;
}

void EFX_RvbProcSrc (openal_channel_t *ch, ALuint source, bool enabled) {
	if (!enabled)
		alSource3i (source, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
	else
		alSource3i (source, AL_AUXILIARY_SEND_FILTER, efx.rvbAuxSlot, 0, AL_FILTER_NULL);
}