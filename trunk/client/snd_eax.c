/*

 Copyright (C) 2007, team Q2XP.

 All of the following routines are simple wrappers to setting EAX buffer properties.

*/
#include "client.h"
#include "snd_loc.h"

qboolean alSource_EAX_All(ALuint sourceNum, LPEAXBUFFERPROPERTIES lpData)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_ALLPARAMETERS,
						sourceNum, lpData, sizeof(EAXBUFFERPROPERTIES));

	return FALSE;
}


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


qboolean alSource_EAX_Flags(ALuint sourceNum, DWORD dwValue)
{
	if (alEAXSet)
		return alEAXSet(&DSPROPSETID_EAX_BufferProperties,
						DSPROPERTY_EAXBUFFER_FLAGS,
						sourceNum, &dwValue, sizeof(DWORD));

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
