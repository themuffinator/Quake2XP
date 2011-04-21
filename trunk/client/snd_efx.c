/*
 Copyright (C) 2006-2007 Team Blur.
 Copyright (C) 2007, team Q2XP.
*/

#include "client.h"
#include "snd_loc.h"

#define MAX_REVERBS 4

typedef struct {
	float density;
	float diffusion;

	float gain;
	float gainHF;
	float gainLF;

	float decayTime;
	float decayHFRatio;
	float decayLFRatio;
	qboolean decayHFLimit;

	float reflectionsGain;
	float reflectionsDelay;
	vec3_t reflectionsPan;

	float lateReverbGain;
	float lateReverbDelay;
	vec3_t lateReverbPan;

	float echoTime;
	float echoDepth;

	float modulationTime;
	float modulationDepth;

	float hfReference;
	float lfReference;

	float airAbsorptionGainHF;
	float roomRolloffFactor;
} reverb_t;

typedef struct {
	qboolean occlusion;
	qboolean reverb;

	qboolean reverbEditing;
	int reverbEditingArea;

	reverb_t reverbGeneric;
	reverb_t reverbUnderwater;

	char reverbName[MAX_OSPATH];
	reverb_t reverbList[MAX_REVERBS];

	unsigned int directFilterId;
	unsigned int reverbFilterId;

	unsigned int effectId;
	unsigned int effectSlotId;
} efx_t;


static reverb_t s_reverbGeneric;	// = {1.0f, 1.0f, 0.32f, 0.89f, 1.0f,
									// 1.49f, 0.83f, 1.0f, AL_TRUE, 0.05f, 
									// 0.007f, odVec3(0.0f, 0.0f, 0.0f),
									// 1.26f, 0.011f, odVec3(0.0f, 0.0f,
									// 0.0f), 0.25f, 0.0f, 0.25f, 0.0f,
									// 5000.0f, 250.0f, 0.994f, 0.0f};
static reverb_t s_reverbUnderwater;	// = {0.3645f, 1.0f, 0.32f, 0.01f,
									// 1.0f, 1.49f, 0.1f, 1.0f, AL_TRUE,
									// 0.596348f, 0.007f, odVec3(0.0f,
									// 0.0f, 0.0f), 7.07946f, 0.011f,
									// odVec3(0.0f, 0.0f, 0.0f), 0.25f,
									// 0.0f, 1.18f, 0.348f, 5000.0f,
									// 250.0f, 0.994f, 0.0f};

efx_t efx;
/*
 ==============================================================================

 ENVIRONMENTAL REVERBERATION EFFECTS PARSING

 ==============================================================================
*/


/*
 ==================
 S_ParseReverb
 ==================
*/
/*static bool S_ParseReverb (script_t *script){

	reverb_t	*reverb;
	token_t		token;
	int			area;
	int			i;

	// Parse the area number
	if (!PS_ExpectTokenType(script, &token, TT_NUMBER, NT_INTEGER)){
		Com_Printf(S_COLOR_YELLOW "WARNING: expected area number, found '%s' instead in reverb file\n", token.string);
		return false;
	}

	area = token.signedIntValue;

	if (area < 0 || area >= CM_NumAreas()){
		Com_Printf(S_COLOR_YELLOW "WARNING: invalid value of %i for area number in reverb file\n", area);
		return false;
	}

	reverb = &snd.efx.reverbList[area];

	// Parse the reverb parameters
	if (!PS_ExpectTokenString(script, &token, "{")){
		Com_Printf(S_COLOR_YELLOW "WARNING: expected '{', found '%s' instead in reverb file (area %i)\n", token.string, area);
		return false;
	}

	while (1){
		if (!PS_ReadToken(script, &token)){
			Com_Printf(S_COLOR_YELLOW "WARNING: no concluding '}' in reverb file (area %i)\n", area);
			return false;	// End of data
		}

		if (!Str_ICompare(token.string, "}"))
			return true;	// End of reverb

		// Parse the parameter
		if (!Str_ICompare(token.string, "density")){
			if (!PS_ReadFloat(script, &reverb->density)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'density' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->density = M_ClampFloat(reverb->density, AL_EAXREVERB_MIN_DENSITY, AL_EAXREVERB_MAX_DENSITY);
		}
		else if (!Str_ICompare(token.string, "diffusion")){
			if (!PS_ReadFloat(script, &reverb->diffusion)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'diffusion' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->diffusion = M_ClampFloat(reverb->diffusion, AL_EAXREVERB_MIN_DIFFUSION, AL_EAXREVERB_MAX_DIFFUSION);
		}
		else if (!Str_ICompare(token.string, "gain")){
			if (!PS_ReadFloat(script, &reverb->gain)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'gain' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->gain = M_ClampFloat(reverb->gain, AL_EAXREVERB_MIN_GAIN, AL_EAXREVERB_MAX_GAIN);
		}
		else if (!Str_ICompare(token.string, "gainHF")){
			if (!PS_ReadFloat(script, &reverb->gainHF)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'gainHF' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->gainHF = M_ClampFloat(reverb->gainHF, AL_EAXREVERB_MIN_GAINHF, AL_EAXREVERB_MAX_GAINHF);
		}
		else if (!Str_ICompare(token.string, "gainLF")){
			if (!PS_ReadFloat(script, &reverb->gainLF)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'gainLF' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->gainLF = M_ClampFloat(reverb->gainLF, AL_EAXREVERB_MIN_GAINLF, AL_EAXREVERB_MAX_GAINLF);
		}
		else if (!Str_ICompare(token.string, "decayTime")){
			if (!PS_ReadFloat(script, &reverb->decayTime)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'decayTime' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->decayTime = M_ClampFloat(reverb->decayTime, AL_EAXREVERB_MIN_DECAY_TIME, AL_EAXREVERB_MAX_DECAY_TIME);
		}
		else if (!Str_ICompare(token.string, "decayHFRatio")){
			if (!PS_ReadFloat(script, &reverb->decayHFRatio)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'decayHFRatio' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->decayHFRatio = M_ClampFloat(reverb->decayHFRatio, AL_EAXREVERB_MIN_DECAY_HFRATIO, AL_EAXREVERB_MAX_DECAY_HFRATIO);
		}
		else if (!Str_ICompare(token.string, "decayLFRatio")){
			if (!PS_ReadFloat(script, &reverb->decayLFRatio)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'decayLFRatio' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->decayLFRatio = M_ClampFloat(reverb->decayLFRatio, AL_EAXREVERB_MIN_DECAY_LFRATIO, AL_EAXREVERB_MAX_DECAY_LFRATIO);
		}
		else if (!Str_ICompare(token.string, "decayHFLimit")){
			if (!PS_ReadSignedInt(script, &reverb->decayHFLimit)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'decayHFLimit' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->decayHFLimit = M_ClampLong(reverb->decayHFLimit, AL_EAXREVERB_MIN_DECAY_HFLIMIT, AL_EAXREVERB_MAX_DECAY_HFLIMIT);
		}
		else if (!Str_ICompare(token.string, "reflectionsGain")){
			if (!PS_ReadFloat(script, &reverb->reflectionsGain)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'reflectionsGain' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->reflectionsGain = M_ClampFloat(reverb->reflectionsGain, AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN);
		}
		else if (!Str_ICompare(token.string, "reflectionsDelay")){
			if (!PS_ReadFloat(script, &reverb->reflectionsDelay)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'reflectionsDelay' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->reflectionsDelay = M_ClampFloat(reverb->reflectionsDelay, AL_EAXREVERB_MIN_REFLECTIONS_DELAY, AL_EAXREVERB_MAX_REFLECTIONS_DELAY);
		}
		else if (!Str_ICompare(token.string, "reflectionsPan")){
			for (i = 0; i < 3; i++){
				if (!PS_ReadFloat(script, &reverb->reflectionsPan[i])){
					Com_Printf(S_COLOR_YELLOW "WARNING: missing 'reflectionsPan' parameters in reverb file (area %i)\n", area);
					return false;
				}
			}
		}
		else if (!Str_ICompare(token.string, "lateReverbGain")){
			if (!PS_ReadFloat(script, &reverb->lateReverbGain)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'lateReverbGain' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->lateReverbGain = M_ClampFloat(reverb->lateReverbGain, AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN);
		}
		else if (!Str_ICompare(token.string, "lateReverbDelay")){
			if (!PS_ReadFloat(script, &reverb->lateReverbDelay)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'lateReverbDelay' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->lateReverbDelay = M_ClampFloat(reverb->lateReverbDelay, AL_EAXREVERB_MIN_LATE_REVERB_DELAY, AL_EAXREVERB_MAX_LATE_REVERB_DELAY);
		}
		else if (!Str_ICompare(token.string, "lateReverbPan")){
			for (i = 0; i < 3; i++){
				if (!PS_ReadFloat(script, &reverb->lateReverbPan[i])){
					Com_Printf(S_COLOR_YELLOW "WARNING: missing 'lateReverbPan' parameters in reverb file (area %i)\n", area);
					return false;
				}
			}
		}
		else if (!Str_ICompare(token.string, "echoTime")){
			if (!PS_ReadFloat(script, &reverb->echoTime)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'echoTime' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->echoTime = M_ClampFloat(reverb->echoTime, AL_EAXREVERB_MIN_ECHO_TIME, AL_EAXREVERB_MAX_ECHO_TIME);
		}
		else if (!Str_ICompare(token.string, "echoDepth")){
			if (!PS_ReadFloat(script, &reverb->echoDepth)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'echoDepth' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->echoDepth = M_ClampFloat(reverb->echoDepth, AL_EAXREVERB_MIN_ECHO_DEPTH, AL_EAXREVERB_MAX_ECHO_DEPTH);
		}
		else if (!Str_ICompare(token.string, "modulationTime")){
			if (!PS_ReadFloat(script, &reverb->modulationTime)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'modulationTime' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->modulationTime = M_ClampFloat(reverb->modulationTime, AL_EAXREVERB_MIN_MODULATION_TIME, AL_EAXREVERB_MAX_MODULATION_TIME);
		}
		else if (!Str_ICompare(token.string, "modulationDepth")){
			if (!PS_ReadFloat(script, &reverb->modulationDepth)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'modulationDepth' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->modulationDepth = M_ClampFloat(reverb->modulationDepth, AL_EAXREVERB_MIN_MODULATION_DEPTH, AL_EAXREVERB_MAX_MODULATION_DEPTH);
		}
		else if (!Str_ICompare(token.string, "hfReference")){
			if (!PS_ReadFloat(script, &reverb->hfReference)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'hfReference' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->hfReference = M_ClampFloat(reverb->hfReference, AL_EAXREVERB_MIN_HFREFERENCE, AL_EAXREVERB_MAX_HFREFERENCE);
		}
		else if (!Str_ICompare(token.string, "lfReference")){
			if (!PS_ReadFloat(script, &reverb->lfReference)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'lfReference' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->lfReference = M_ClampFloat(reverb->lfReference, AL_EAXREVERB_MIN_LFREFERENCE, AL_EAXREVERB_MAX_LFREFERENCE);
		}
		else if (!Str_ICompare(token.string, "airAbsorptionGainHF")){
			if (!PS_ReadFloat(script, &reverb->airAbsorptionGainHF)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'airAbsorptionGainHF' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->airAbsorptionGainHF = M_ClampFloat(reverb->airAbsorptionGainHF, AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF);
		}
		else if (!Str_ICompare(token.string, "roomRolloffFactor")){
			if (!PS_ReadFloat(script, &reverb->roomRolloffFactor)){
				Com_Printf(S_COLOR_YELLOW "WARNING: missing 'roomRolloffFactor' parameter in reverb file (area %i)\n", area);
				return false;
			}

			reverb->roomRolloffFactor = M_ClampFloat(reverb->roomRolloffFactor, AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR, AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR);
		}
		else {
			Com_Printf(S_COLOR_YELLOW "WARNING: unknown parameter '%s' in reverb file (area %i)\n", token.string, area);
			return false;
		}
	}

	return true;
}*/

/*
 ==================
 S_LoadReverbs
 ==================
*/
/*void S_LoadReverbs (const char *name){

	script_t	*script;
	token_t		token;

	// Copy the name
	Str_Copy(snd.efx.reverbName, name, sizeof(snd.efx.reverbName));

	// Load the script file
	script = PS_LoadScriptFile(name);
	if (!script){
		Com_Printf("Reverb file %s not found\n", name);
		return;
	}

	PS_SetScriptFlags(script, SF_NOWARNINGS | SF_NOERRORS);

	// Parse it
	while (1){
		if (!PS_ReadToken(script, &token))
			break;		// End of data

		if (!Str_ICompare(token.string, "reverb")){
			if (!S_ParseReverb(script))
				break;
		}
		else {
			Com_Printf(S_COLOR_YELLOW "WARNING: expected 'reverb', found '%s' instead in reverb file\n", token.string);
			break;
		}
	}

	// Free the script file
	PS_FreeScript(script);
}*/


/*
 ==============================================================================

 INTEGRATED ENVIRONMENTAL REVERBERATION EFFECTS EDITOR

 ==============================================================================
*/


/*
 ==================
 S_EditCurrentReverb
 ==================
*/
/*void S_EditCurrentReverb (void){

	reverb_t			*reverb;
	reverbProperties_t	properties;

	if (!snd.efx.reverbEditing)
		return;		// Not active

	if (snd.efx.reverbEditingArea == snd.listener.area)
		return;		// Area hasn't changed
	snd.efx.reverbEditingArea = snd.listener.area;

	// The area has changed, so edit the current reverb properties
	if (snd.efx.reverbEditingArea == -1){
		ED_EditReverbProperties(-1, NULL);
		return;
	}

	reverb = &snd.efx.reverbList[snd.efx.reverbEditingArea];

	properties.density = reverb->density;
	properties.diffusion = reverb->diffusion;
	properties.gain = reverb->gain;
	properties.gainHF = reverb->gainHF;
	properties.gainLF = reverb->gainLF;
	properties.decayTime = reverb->decayTime;
	properties.decayHFRatio = reverb->decayHFRatio;
	properties.decayLFRatio = reverb->decayLFRatio;
	properties.decayHFLimit = reverb->decayHFLimit;
	properties.reflectionsGain = reverb->reflectionsGain;
	properties.reflectionsDelay = reverb->reflectionsDelay;
	properties.reflectionsPan = reverb->reflectionsPan;
	properties.lateReverbGain = reverb->lateReverbGain;
	properties.lateReverbDelay = reverb->lateReverbDelay;
	properties.lateReverbPan = reverb->lateReverbPan;
	properties.echoTime = reverb->echoTime;
	properties.echoDepth = reverb->echoDepth;
	properties.modulationTime = reverb->modulationTime;
	properties.modulationDepth = reverb->modulationDepth;
	properties.hfReference = reverb->hfReference;
	properties.lfReference = reverb->lfReference;
	properties.airAbsorptionGainHF = reverb->airAbsorptionGainHF;
	properties.roomRolloffFactor = reverb->roomRolloffFactor;

	ED_EditReverbProperties(snd.efx.reverbEditingArea, &properties);
}*/

/*
 ==================
 S_ReverbEditorUpdateCallback
 ==================
*/
/*static void S_ReverbEditorUpdateCallback (int index, reverbProperties_t *properties){

	reverb_t	*reverb;

	if (!snd.efx.reverbEditing)
		return;		// Not active

	if (index < 0 || index >= CM_NumAreas())
		return;		// Out of range

	reverb = &snd.efx.reverbList[index];

	// Clamp and copy the properties
	reverb->density = M_ClampFloat(properties->density, AL_EAXREVERB_MIN_DENSITY, AL_EAXREVERB_MAX_DENSITY);
	reverb->diffusion = M_ClampFloat(properties->diffusion, AL_EAXREVERB_MIN_DIFFUSION, AL_EAXREVERB_MAX_DIFFUSION);
	reverb->gain = M_ClampFloat(properties->gain, AL_EAXREVERB_MIN_GAIN, AL_EAXREVERB_MAX_GAIN);
	reverb->gainHF = M_ClampFloat(properties->gainHF, AL_EAXREVERB_MIN_GAINHF, AL_EAXREVERB_MAX_GAINHF);
	reverb->gainLF = M_ClampFloat(properties->gainLF, AL_EAXREVERB_MIN_GAINLF, AL_EAXREVERB_MAX_GAINLF);
	reverb->decayTime = M_ClampFloat(properties->decayTime, AL_EAXREVERB_MIN_DECAY_TIME, AL_EAXREVERB_MAX_DECAY_TIME);
	reverb->decayHFRatio = M_ClampFloat(properties->decayHFRatio, AL_EAXREVERB_MIN_DECAY_HFRATIO, AL_EAXREVERB_MAX_DECAY_HFRATIO);
	reverb->decayLFRatio = M_ClampFloat(properties->decayLFRatio, AL_EAXREVERB_MIN_DECAY_LFRATIO, AL_EAXREVERB_MAX_DECAY_LFRATIO);
	reverb->decayHFLimit = M_ClampLong(properties->decayHFLimit, AL_EAXREVERB_MIN_DECAY_HFLIMIT, AL_EAXREVERB_MAX_DECAY_HFLIMIT);
	reverb->reflectionsGain = M_ClampFloat(properties->reflectionsGain, AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN);
	reverb->reflectionsDelay = M_ClampFloat(properties->reflectionsDelay, AL_EAXREVERB_MIN_REFLECTIONS_DELAY, AL_EAXREVERB_MAX_REFLECTIONS_DELAY);
	reverb->reflectionsPan = properties->reflectionsPan;
	reverb->lateReverbGain = M_ClampFloat(properties->lateReverbGain, AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN);
	reverb->lateReverbDelay = M_ClampFloat(properties->lateReverbDelay, AL_EAXREVERB_MIN_LATE_REVERB_DELAY, AL_EAXREVERB_MAX_LATE_REVERB_DELAY);
	reverb->lateReverbPan = properties->lateReverbPan;
	reverb->echoTime = M_ClampFloat(properties->echoTime, AL_EAXREVERB_MIN_ECHO_TIME, AL_EAXREVERB_MAX_ECHO_TIME);
	reverb->echoDepth = M_ClampFloat(properties->echoDepth, AL_EAXREVERB_MIN_ECHO_DEPTH, AL_EAXREVERB_MAX_ECHO_DEPTH);
	reverb->modulationTime = M_ClampFloat(properties->modulationTime, AL_EAXREVERB_MIN_MODULATION_TIME, AL_EAXREVERB_MAX_MODULATION_TIME);
	reverb->modulationDepth = M_ClampFloat(properties->modulationDepth, AL_EAXREVERB_MIN_MODULATION_DEPTH, AL_EAXREVERB_MAX_MODULATION_DEPTH);
	reverb->hfReference = M_ClampFloat(properties->hfReference, AL_EAXREVERB_MIN_HFREFERENCE, AL_EAXREVERB_MAX_HFREFERENCE);
	reverb->lfReference = M_ClampFloat(properties->lfReference, AL_EAXREVERB_MIN_LFREFERENCE, AL_EAXREVERB_MAX_LFREFERENCE);
	reverb->airAbsorptionGainHF = M_ClampFloat(properties->airAbsorptionGainHF, AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF);
	reverb->roomRolloffFactor = M_ClampFloat(properties->roomRolloffFactor, AL_EAXREVERB_MIN_ROOM_ROLLOFF_FACTOR, AL_EAXREVERB_MAX_ROOM_ROLLOFF_FACTOR);

	// Force the listener to update the reverb
	snd.listener.reverb = NULL;
}*/

/*
 ==================
 S_ReverbEditorClearCallback
 ==================
*/
/*static void S_ReverbEditorClearCallback (void){

	int		i;

	if (!snd.efx.reverbEditing)
		return;		// Not active

	// Fill in defaults
	for (i = 0; i < CM_NumAreas(); i++)
		snd.efx.reverbList[i] = s_reverbGeneric;

	// Force the listener to update the reverb
	snd.listener.reverb = NULL;
}*/

/*
 ==================
 S_ReverbEditorSaveCallback
 ==================
*/
/*static void S_ReverbEditorSaveCallback (void){

	fileHandle_t	f;
	reverb_t		*reverb;
	int				i;

	if (!snd.efx.reverbEditing)
		return;		// Not active

	if (!snd.efx.reverbName[0]){
		Com_Printf("No reverb file name available\n");
		return;
	}

	// Write the script file
	FS_OpenFile(snd.efx.reverbName, &f, FS_WRITE);
	if (!f){
		Com_Printf("Couldn't write %s\n", snd.efx.reverbName);
		return;
	}

	for (i = 0, reverb = snd.efx.reverbList; i < CM_NumAreas(); i++, reverb++){
		FS_Printf(f, "reverb %i" LINE_SEPARATOR, i);
		FS_Printf(f, "{" LINE_SEPARATOR);
		FS_Printf(f, "density               %g" LINE_SEPARATOR, reverb->density);
		FS_Printf(f, "diffusion             %g" LINE_SEPARATOR, reverb->diffusion);
		FS_Printf(f, "gain                  %g" LINE_SEPARATOR, reverb->gain);
		FS_Printf(f, "gainHF                %g" LINE_SEPARATOR, reverb->gainHF);
		FS_Printf(f, "gainLF                %g" LINE_SEPARATOR, reverb->gainLF);
		FS_Printf(f, "decayTime             %g" LINE_SEPARATOR, reverb->decayTime);
		FS_Printf(f, "decayHFRatio          %g" LINE_SEPARATOR, reverb->decayHFRatio);
		FS_Printf(f, "decayLFRatio          %g" LINE_SEPARATOR, reverb->decayLFRatio);
		FS_Printf(f, "decayHFLimit          %i" LINE_SEPARATOR, reverb->decayHFLimit);
		FS_Printf(f, "reflectionsGain       %g" LINE_SEPARATOR, reverb->reflectionsGain);
		FS_Printf(f, "reflectionsDelay      %g" LINE_SEPARATOR, reverb->reflectionsDelay);
		FS_Printf(f, "reflectionsPan        %g %g %g" LINE_SEPARATOR, reverb->reflectionsPan[0], reverb->reflectionsPan[1], reverb->reflectionsPan[2]);
		FS_Printf(f, "lateReverbGain        %g" LINE_SEPARATOR, reverb->lateReverbGain);
		FS_Printf(f, "lateReverbDelay       %g" LINE_SEPARATOR, reverb->lateReverbDelay);
		FS_Printf(f, "lateReverbPan         %g %g %g" LINE_SEPARATOR, reverb->lateReverbPan[0], reverb->lateReverbPan[1], reverb->lateReverbPan[2]);
		FS_Printf(f, "echoTime              %g" LINE_SEPARATOR, reverb->echoTime);
		FS_Printf(f, "echoDepth             %g" LINE_SEPARATOR, reverb->echoDepth);
		FS_Printf(f, "modulationTime        %g" LINE_SEPARATOR, reverb->modulationTime);
		FS_Printf(f, "modulationDepth       %g" LINE_SEPARATOR, reverb->modulationDepth);
		FS_Printf(f, "hfReference           %g" LINE_SEPARATOR, reverb->hfReference);
		FS_Printf(f, "lfReference           %g" LINE_SEPARATOR, reverb->lfReference);
		FS_Printf(f, "airAbsorptionGainHF   %g" LINE_SEPARATOR, reverb->airAbsorptionGainHF);
		FS_Printf(f, "roomRolloffFactor     %g" LINE_SEPARATOR, reverb->roomRolloffFactor);
		FS_Printf(f, "}" LINE_SEPARATOR);

		if (i < CM_NumAreas() - 1)
			FS_Printf(f, LINE_SEPARATOR);
	}

	FS_CloseFile(f);

	Com_Printf("Wrote %s with %i reverbs\n", snd.efx.reverbName, CM_NumAreas());
}*/

/*
 ==================
 S_ReverbEditorCloseCallback
 ==================
*/
/*static void S_ReverbEditorCloseCallback (void){

	if (!snd.efx.reverbEditing)
		return;		// Not active

	snd.efx.reverbEditing = false;
	snd.efx.reverbEditingArea = 0;

	// Close the reverb editor
	ED_CloseEditor();
}*/

/*
 ==================
 S_EditReverbs_f
 ==================
*/
/*static void S_EditReverbs_f (void){

	reverbCallbacks_t	callbacks;

	// Launch the reverb editor
	if (!ED_LaunchEditor(EDITOR_REVERB, NULL, NULL))
		return;

	snd.efx.reverbEditing = true;
	snd.efx.reverbEditingArea = -1;

	// Set the callback functions
	callbacks.Update = S_ReverbEditorUpdateCallback;
	callbacks.Clear = S_ReverbEditorClearCallback;
	callbacks.Save = S_ReverbEditorSaveCallback;
	callbacks.Close = S_ReverbEditorCloseCallback;

	ED_SetReverbCallbacks(&callbacks);

	// Finish setting up, but don't edit anything yet
	ED_EditReverbProperties(-1, NULL);
}*/


// ============================================================================


/*
 ==================
 S_InitEFX
 ==================
*/
void S_InitEFX(void)
{
	int i;
/*
	// Register our commands
	Cmd_AddCommand("editReverbs", S_EditReverbs_f, "Launches the integrated reverb editor");
*/
	// Fill in defaults
	efx.reverbGeneric = s_reverbGeneric;
	efx.reverbUnderwater = s_reverbUnderwater;

	for (i = 0; i < MAX_REVERBS; i++)
		efx.reverbList[i] = s_reverbGeneric;

	if (alConfig.eax == VER_EFX) {
		// Initialize EFX occlusion
		efx.occlusion = true;

		alGenFilters(1, &efx.directFilterId);
		alFilteri(efx.directFilterId, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		alFilterf(efx.directFilterId, AL_LOWPASS_GAIN, 1.0f);
		alFilterf(efx.directFilterId, AL_LOWPASS_GAINHF, 0.1f);

		alGenFilters(1, &efx.reverbFilterId);
		alFilteri(efx.reverbFilterId, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
		alFilterf(efx.reverbFilterId, AL_LOWPASS_GAIN, 0.5f);
		alFilterf(efx.reverbFilterId, AL_LOWPASS_GAINHF, 0.1f);

		// Disable it if an error occurred
		if (alGetError() != AL_NO_ERROR) {
			alFilteri(efx.directFilterId, AL_FILTER_TYPE, 0);
			alDeleteFilters(1, &efx.directFilterId);

			alFilteri(efx.reverbFilterId, AL_FILTER_TYPE, 0);
			alDeleteFilters(1, &efx.reverbFilterId);

			alGetError();

			// Clear the variables
			efx.occlusion = false;

			efx.directFilterId = 0;
			efx.reverbFilterId = 0;
		}
		// Initialize EFX reverb
		efx.reverb = true;

		alGenEffects(1, &efx.effectId);
		alEffecti(efx.effectId, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
		alEffectf(efx.effectId, AL_EAXREVERB_DENSITY,
				  s_reverbGeneric.density);
		alEffectf(efx.effectId, AL_EAXREVERB_DIFFUSION,
				  s_reverbGeneric.diffusion);
		alEffectf(efx.effectId, AL_EAXREVERB_GAIN, s_reverbGeneric.gain);
		alEffectf(efx.effectId, AL_EAXREVERB_GAINHF,
				  s_reverbGeneric.gainHF);
		alEffectf(efx.effectId, AL_EAXREVERB_GAINLF,
				  s_reverbGeneric.gainLF);
		alEffectf(efx.effectId, AL_EAXREVERB_DECAY_TIME,
				  s_reverbGeneric.decayTime);
		alEffectf(efx.effectId, AL_EAXREVERB_DECAY_HFRATIO,
				  s_reverbGeneric.decayHFRatio);
		alEffectf(efx.effectId, AL_EAXREVERB_DECAY_LFRATIO,
				  s_reverbGeneric.decayLFRatio);
		alEffecti(efx.effectId, AL_EAXREVERB_DECAY_HFLIMIT,
				  s_reverbGeneric.decayHFLimit);
		alEffectf(efx.effectId, AL_EAXREVERB_REFLECTIONS_GAIN,
				  s_reverbGeneric.reflectionsGain);
		alEffectf(efx.effectId, AL_EAXREVERB_REFLECTIONS_DELAY,
				  s_reverbGeneric.reflectionsDelay);
		alEffectfv(efx.effectId, AL_EAXREVERB_REFLECTIONS_PAN,
				   /*&*/s_reverbGeneric.reflectionsPan);
		alEffectf(efx.effectId, AL_EAXREVERB_LATE_REVERB_GAIN,
				  s_reverbGeneric.lateReverbGain);
		alEffectf(efx.effectId, AL_EAXREVERB_LATE_REVERB_DELAY,
				  s_reverbGeneric.lateReverbDelay);
		alEffectfv(efx.effectId, AL_EAXREVERB_LATE_REVERB_PAN,
				   /*&*/s_reverbGeneric.lateReverbPan);
		alEffectf(efx.effectId, AL_EAXREVERB_ECHO_TIME,
				  s_reverbGeneric.echoTime);
		alEffectf(efx.effectId, AL_EAXREVERB_ECHO_DEPTH,
				  s_reverbGeneric.echoDepth);
		alEffectf(efx.effectId, AL_EAXREVERB_MODULATION_TIME,
				  s_reverbGeneric.modulationTime);
		alEffectf(efx.effectId, AL_EAXREVERB_MODULATION_DEPTH,
				  s_reverbGeneric.modulationDepth);
		alEffectf(efx.effectId, AL_EAXREVERB_HFREFERENCE,
				  s_reverbGeneric.hfReference);
		alEffectf(efx.effectId, AL_EAXREVERB_LFREFERENCE,
				  s_reverbGeneric.lfReference);
		alEffectf(efx.effectId, AL_EAXREVERB_AIR_ABSORPTION_GAINHF,
				  s_reverbGeneric.airAbsorptionGainHF);
		alEffectf(efx.effectId, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,
				  s_reverbGeneric.roomRolloffFactor);

		alGenAuxiliaryEffectSlots(1, &efx.effectSlotId);
		alAuxiliaryEffectSloti(efx.effectSlotId, AL_EFFECTSLOT_EFFECT,
							   efx.effectId);

		// Disable it if an error occurred
		if (alGetError() != AL_NO_ERROR) {
			alAuxiliaryEffectSloti(efx.effectSlotId, AL_EFFECTSLOT_EFFECT,
								   0);
			alDeleteAuxiliaryEffectSlots(1, &efx.effectSlotId);

			alEffecti(efx.effectId, AL_EFFECT_TYPE, 0);
			alDeleteEffects(1, &efx.effectId);

			alGetError();

			// Clear the variables
			efx.reverb = false;

			efx.effectId = 0;
			efx.effectSlotId = 0;
		}
	}
}

/*
 ==================
 S_ShutdownEFX
 ==================
*/
void S_ShutdownEFX(void)
{

	// Unregister our commands
/*	Cmd_RemoveCommand("editReverbs");*/

	// Close the reverb editor if active
/*	if (snd.efx.reverbEditing)
		ED_CloseEditor();*/

	// Shutdown EFX occlusion
	if (efx.occlusion) {
		alFilteri(efx.directFilterId, AL_FILTER_TYPE, 0);
		alDeleteFilters(1, &efx.directFilterId);

		alFilteri(efx.reverbFilterId, AL_FILTER_TYPE, 0);
		alDeleteFilters(1, &efx.reverbFilterId);
	}
	// Shutdown EFX reverb
	if (efx.reverb) {
		alAuxiliaryEffectSloti(efx.effectSlotId, AL_EFFECTSLOT_EFFECT, 0);
		alDeleteAuxiliaryEffectSlots(1, &efx.effectSlotId);

		alEffecti(efx.effectId, AL_EFFECT_TYPE, 0);
		alDeleteEffects(1, &efx.effectId);
	}
}
