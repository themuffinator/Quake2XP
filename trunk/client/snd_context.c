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

#include "../client/client.h"
#include "../client/snd_loc.h"

alConfig_t alConfig;
qboolean	openalStop = false;
/*
 =================
 AL_InitDriver
 =================
*/
static qboolean AL_InitDriver(void)
{
	char *deviceName = s_openal_device->string;

	Com_DPrintf("Initializing OpenAL driver\n");

	if (!deviceName[0])
		deviceName = NULL;

	if (deviceName)
		Com_Printf("...opening device ("S_COLOR_GREEN"%s"S_COLOR_WHITE"): ", deviceName);
	else
		Com_Printf("...opening default device: ");

	// Open the device
	if ((alConfig.hDevice = alcOpenDevice(deviceName)) == NULL) {
		Com_Printf("failed\n");
		return false;
	}

	if (!deviceName)
		Com_Printf("succeeded ("S_COLOR_GREEN"%s"S_COLOR_WHITE")\n",
				   alcGetString(alConfig.hDevice, ALC_DEVICE_SPECIFIER));
	else
		Com_Printf("succeeded\n");

	// Create the AL context and make it current
	Com_DPrintf("...creating AL context: ");
	{
		// Setup context attributes at context creation time:
		// ALC_FREQUENCY, ALC_REFRESH, ALC_SYNC, ALC_MONO_SOURCES,
		// ALC_STEREO_SOURCES
		ALCint attrlist[3] = { ALC_FREQUENCY, 48000, 0 };
		ALCint *attrlist_p;

		if (!(s_quality->value))
			attrlist_p = NULL;
		else {
			switch ((int) s_quality->value) {
			case 1:
				attrlist[1] = 44100;
			case 2:
				attrlist[1] = 48000;
			case 3:
				attrlist[1] = 88200;
			case 4:
				attrlist[1] = 96000;
			case 5:
				attrlist[1] = 176400;
			case 6:
				attrlist[1] = 192000;
			}
			attrlist_p = attrlist;
		}


		if ((alConfig.hALC =
			 alcCreateContext(alConfig.hDevice, attrlist_p)) == NULL) {
			Com_DPrintf("failed\n");
			goto failed;
		}
	}
	Com_DPrintf("succeeded\n");

	Com_DPrintf("...making context current: ");
	if (!alcMakeContextCurrent(alConfig.hALC)) {
		Com_DPrintf("failed\n");
		goto failed;
	}
	Com_DPrintf("succeeded\n");

	return true;

  failed:

	Com_Printf(S_COLOR_RED"...failed hard\n");
	
	openalStop = true;

	if (alConfig.hALC) {
		alcDestroyContext(alConfig.hALC);
		alConfig.hALC = NULL;
	}

	if (alConfig.hDevice) {
		alcCloseDevice(alConfig.hDevice);
		alConfig.hDevice = NULL;
	}

	return false;
}

/*
 =================
 AL_StartOpenAL
 =================
*/
qboolean AL_StartOpenAL(void)
{
	extern const char *al_device[];

	// Get device list
	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
		unsigned i = 0;
		const char *a = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
		if (!a)					// We have no audio output devices. No
								// hope.
		{
			QAL_Shutdown();
			return false;
		}
		while (*a) {
			al_device[++i] = a;
			Com_DPrintf("Device species %i: %s\n", i, a);
			while (*a)
				a++;
			a++;
		}
		alConfig.device_count = i;
	} else {
		// OS Specific devices. We are here, because device enumeration
		// failed!
#ifdef _WIN32
		al_device[1] = "DirectSound3D";
		al_device[2] = "DirectSound";
		al_device[3] = "MMSYSTEM";
		alConfig.device_count = 3;
#elif
		QAL_Shutdown();
		return false;
#endif
	}

	// Initialize the device, context, etc...
	if (AL_InitDriver())
		return true;

	// Shutdown QAL
	QAL_Shutdown();

	return false;
}

/*
 =================
 AL_Shutdown
 =================
*/
void AL_Shutdown(void)
{
	Com_Printf("Shutting down OpenAL subsystem\n");

	if (alConfig.hALC) {
		if (alcMakeContextCurrent) {
			Com_Printf("...alcMakeContextCurrent( NULL ): ");
			if (!alcMakeContextCurrent(NULL))
				Com_Printf("failed\n");
			else
				Com_Printf("succeeded\n");
		}

		if (alcDestroyContext) {
			Com_Printf("...destroying AL context\n");
			alcDestroyContext(alConfig.hALC);
		}

		alConfig.hALC = NULL;
	}

	if (alConfig.hDevice) {
		if (alcCloseDevice) {
			Com_Printf("...closing device\n");
			alcCloseDevice(alConfig.hDevice);
		}

		alConfig.hDevice = NULL;
	}

	QAL_Shutdown();
}
