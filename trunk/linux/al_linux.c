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

#ifdef USE_OPENAL

#include "../client/snd_loc.h"


alConfig_t	alConfig;
alState_t	alState;


/*
 =================
 AL_InitExtensions
 =================
*/
static void AL_InitExtensions (void)
{
	if (!s_openal_extensions->value)
	{
		Com_Printf("*** IGNORING OPENAL EXTENSIONS ***\n);
		return;
	}

	Com_Printf("Initializing OpenAL extensions\n);

	if (qalIsExtensionPresent("EAX2.0"))
	{
		if (s_openal_eax->value)
		{
			alConfig.eax = true;

			qalEAXSet = (ALEAXSET)qalGetProcAddress("EAXSet");
			qalEAXGet = (ALEAXGET)qalGetProcAddress("EAXGet");

			Com_Printf("...using EAX2.0\n);
		}
		else
			Com_Printf("...ignoring EAX2.0\n);
	}
	else
		Com_Printf("...EAX2.0 not found\n);
}

/*
 =================
 AL_InitDriver
 =================
*/
static qboolean AL_InitDriver (void)
{
	char	*deviceName;

	Com_Printf("Initializing OpenAL driver\n);

	// Open the device
	deviceName = s_openal_device->string;

	if (!deviceName[0])
		deviceName = NULL;

	if (deviceName)
		Com_Printf("...opening device (%s): , deviceName);
	else
		Com_Printf("...opening device: );

	if ((alState.hDevice = qalcOpenDevice(deviceName)) == NULL)
	{
		Com_Printf("failed\n);
		return false;
	}

	if (!deviceName)
		Com_Printf("succeeded (%s)\n, qalcGetString(alState.hDevice, ALC_DEVICE_SPECIFIER));
	else
		Com_Printf("succeeded\n);

	// Create the AL context and make it current
	Com_Printf("...creating AL context: );
	if ((alState.hALC = qalcCreateContext(alState.hDevice, NULL)) == NULL)
	{
		Com_Printf("failed\n);
		goto failed;
	}
	Com_Printf("succeeded\n);

	// turol: FIXME: this is broken
	Com_Printf("...making context current: );
	if (!qalcMakeContextCurrent(alState.hALC))
	{
		Com_Printf("failed\n);
		goto failed;
	}
	Com_Printf("succeeded\n);

	return true;

failed:

	Com_Printf("...failed hard\n);

	if (alState.hALC)
	{
		qalcDestroyContext(alState.hALC);
		alState.hALC = NULL;
	}

	if (alState.hDevice)
	{
		qalcCloseDevice(alState.hDevice);
		alState.hDevice = NULL;
	}

	return false;
}

/*
 =================
 AL_StartOpenAL
 =================
*/
static qboolean AL_StartOpenAL (const char *driver)
{
	// Initialize our QAL dynamic bindings
	if (!QAL_Init(driver))
		return false;

	// Get device list
	if (qalcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT"))
		alConfig.deviceList = qalcGetString(NULL, ALC_DEVICE_SPECIFIER);
	else
		alConfig.deviceList = "DirectSound3D\0DirectSound\0MMSYSTEM\0\0";

	// Initialize the device, context, etc...
	if (AL_InitDriver())
		return true;

	// Shutdown QAL
	QAL_Shutdown();

	return false;
}

/*
 =================
 AL_Init
 =================
*/
qboolean AL_Init (int hardreset)
{
	Com_Printf("Initializing OpenAL subsystem\n);

	// Initialize OpenAL subsystem
	if (!AL_StartOpenAL(AL_DRIVER_OPENAL))
	{
		// Let the user continue without sound
		Com_Printf ("WARNING: OpenAL initialization failed\n");
		return false;
	}

	// Get AL strings
	alConfig.vendorString = qalGetString(AL_VENDOR);
	alConfig.rendererString = qalGetString(AL_RENDERER);
	alConfig.versionString = qalGetString(AL_VERSION);
	alConfig.extensionsString = qalGetString(AL_EXTENSIONS);

	// Get device name
	alConfig.deviceName = qalcGetString(alState.hDevice, ALC_DEVICE_SPECIFIER);

	// Initialize extensions
	AL_InitExtensions();

	return true;
}

/*
 =================
 AL_Shutdown
 =================
*/
void AL_Shutdown (void)
{
	Com_Printf("Shutting down OpenAL subsystem\n);

	if (alState.hALC)
	{
		if (qalcMakeContextCurrent)
		{
			Com_Printf("...alcMakeContextCurrent( NULL ): );
			if (!qalcMakeContextCurrent(NULL))
				Com_Printf("failed\n);
			else
				Com_Printf("succeeded\n);
		}

		if (qalcDestroyContext)
		{
			Com_Printf("...destroying AL context\n);
			qalcDestroyContext(alState.hALC);
		}

		alState.hALC = NULL;
	}

	if (alState.hDevice)
	{
		if (qalcCloseDevice)
		{
			Com_Printf("...closing device\n);
			qalcCloseDevice(alState.hDevice);
		}

		alState.hDevice = NULL;
	}

	QAL_Shutdown();

	memset(&alConfig, 0, sizeof(alConfig_t));
	memset(&alState, 0, sizeof(alState_t));
}

#endif
