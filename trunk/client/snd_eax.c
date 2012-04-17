/*

 Copyright (C) 2007, team Q2XP.

*/
#include "client.h"
#include "snd_loc.h"

const GUID DSPROPSETID_EAX20_ListenerProperties =
	{ 0x306a6a8, 0xb224, 0x11d2, {0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7,
								  0x22} };
const GUID DSPROPSETID_EAX20_BufferProperties =
	{ 0x306a6a7, 0xb224, 0x11d2, {0x99, 0xe5, 0x0, 0x0, 0xe8, 0xd8, 0xc7,
								  0x22} };

void applyEAX_Effects(void)
{
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

qboolean alSource_EAX_All(ALuint sourceNum, LPEAXBUFFERPROPERTIES lpData);
qboolean alSource_EAX_Flags(ALuint sourceNum, DWORD dwValue);

void normalEAX_Effects(openal_channel_t *ch, ALuint sourceNum)
{
	EAXBUFFERPROPERTIES normalEAX;

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
}

void nullEAX_Effects(openal_channel_t *ch, ALuint sourceNum)
{
	EAXBUFFERPROPERTIES nullEAX;

	// alSource_EAX_Flags(sourceNum, 0);
	// willow: should be no EAX effects!
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
	// alEAXSet(&DSPROPSETID_EAX_BufferProperties, DSPROPERTY_EAXBUFFER_NONE, sourceNum, 0, 0);
}

// All of the following routines are simple wrappers to setting EAX buffer properties.

qboolean alSource_EAX_All(ALuint sourceNum, LPEAXBUFFERPROPERTIES lpData)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_ALLPARAMETERS,
						sourceNum, lpData, sizeof(EAXBUFFERPROPERTIES));

	return FALSE;
}


qboolean alSource_EAX_Flags(ALuint sourceNum, DWORD dwValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_FLAGS,
						sourceNum, &dwValue, sizeof(DWORD));

	return FALSE;
}


#if 0
qboolean alSource_EAX_Direct(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_DIRECT,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_DirectHF(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_DIRECTHF,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_Room(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_ROOM,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_RoomHF(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_ROOMHF,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_Rolloff(ALuint sourceNum, float fValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_ROOMROLLOFFFACTOR,
						sourceNum, &fValue, sizeof(float));

	return FALSE;
}


qboolean alSource_EAX_Outside(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_OUTSIDEVOLUMEHF,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_Absorption(ALuint sourceNum, float fValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_AIRABSORPTIONFACTOR,
						sourceNum, &fValue, sizeof(float));

	return FALSE;
}


qboolean alSource_EAX_Obstruction(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_OBSTRUCTION,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_ObstructionLF(ALuint sourceNum, float fValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_OBSTRUCTIONLFRATIO,
						sourceNum, &fValue, sizeof(float));

	return FALSE;
}


qboolean alSource_EAX_Occlusion(ALuint sourceNum, LONG lValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_OCCLUSION,
						sourceNum, &lValue, sizeof(LONG));

	return FALSE;
}


qboolean alSource_EAX_OcclusionLF(ALuint sourceNum, float fValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_OCCLUSIONLFRATIO,
						sourceNum, &fValue, sizeof(float));

	return FALSE;
}


qboolean alSource_EAX_OcclusionRoom(ALuint sourceNum, float fValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_OCCLUSIONROOMRATIO,
						sourceNum, &fValue, sizeof(float));

	return FALSE;
}
#endif
