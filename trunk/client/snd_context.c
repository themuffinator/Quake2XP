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

#include "../client/client.h"
#include "../client/snd_loc.h"

alConfig_t alConfig;
qboolean	openalStop = qfalse;

#ifdef _WIN32
extern LPALGETSTRINGISOFT alGetStringiSOFT;
#endif

typedef struct ConvLetter {
	char    win1251;
	int     unicode;
} Letter;


static Letter g_letters[] = {
		{0x82, 0x201A}, // SINGLE LOW-9 QUOTATION MARK
		{0x83, 0x0453}, // CYRILLIC SMALL LETTER GJE
		{0x84, 0x201E}, // DOUBLE LOW-9 QUOTATION MARK
		{0x85, 0x2026}, // HORIZONTAL ELLIPSIS
		{0x86, 0x2020}, // DAGGER
		{0x87, 0x2021}, // DOUBLE DAGGER
		{0x88, 0x20AC}, // EURO SIGN
		{0x89, 0x2030}, // PER MILLE SIGN
		{0x8A, 0x0409}, // CYRILLIC CAPITAL LETTER LJE
		{0x8B, 0x2039}, // SINGLE LEFT-POINTING ANGLE QUOTATION MARK
		{0x8C, 0x040A}, // CYRILLIC CAPITAL LETTER NJE
		{0x8D, 0x040C}, // CYRILLIC CAPITAL LETTER KJE
		{0x8E, 0x040B}, // CYRILLIC CAPITAL LETTER TSHE
		{0x8F, 0x040F}, // CYRILLIC CAPITAL LETTER DZHE
		{0x90, 0x0452}, // CYRILLIC SMALL LETTER DJE
		{0x91, 0x2018}, // LEFT SINGLE QUOTATION MARK
		{0x92, 0x2019}, // RIGHT SINGLE QUOTATION MARK
		{0x93, 0x201C}, // LEFT DOUBLE QUOTATION MARK
		{0x94, 0x201D}, // RIGHT DOUBLE QUOTATION MARK
		{0x95, 0x2022}, // BULLET
		{0x96, 0x2013}, // EN DASH
		{0x97, 0x2014}, // EM DASH
		{0x99, 0x2122}, // TRADE MARK SIGN
		{0x9A, 0x0459}, // CYRILLIC SMALL LETTER LJE
		{0x9B, 0x203A}, // SINGLE RIGHT-POINTING ANGLE QUOTATION MARK
		{0x9C, 0x045A}, // CYRILLIC SMALL LETTER NJE
		{0x9D, 0x045C}, // CYRILLIC SMALL LETTER KJE
		{0x9E, 0x045B}, // CYRILLIC SMALL LETTER TSHE
		{0x9F, 0x045F}, // CYRILLIC SMALL LETTER DZHE
		{0xA0, 0x00A0}, // NO-BREAK SPACE
		{0xA1, 0x040E}, // CYRILLIC CAPITAL LETTER SHORT U
		{0xA2, 0x045E}, // CYRILLIC SMALL LETTER SHORT U
		{0xA3, 0x0408}, // CYRILLIC CAPITAL LETTER JE
		{0xA4, 0x00A4}, // CURRENCY SIGN
		{0xA5, 0x0490}, // CYRILLIC CAPITAL LETTER GHE WITH UPTURN
		{0xA6, 0x00A6}, // BROKEN BAR
		{0xA7, 0x00A7}, // SECTION SIGN
		{0xA8, 0x0401}, // CYRILLIC CAPITAL LETTER IO
		{0xA9, 0x00A9}, // COPYRIGHT SIGN
		{0xAA, 0x0404}, // CYRILLIC CAPITAL LETTER UKRAINIAN IE
		{0xAB, 0x00AB}, // LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
		{0xAC, 0x00AC}, // NOT SIGN
		{0xAD, 0x00AD}, // SOFT HYPHEN
		{0xAE, 0x00AE}, // REGISTERED SIGN
		{0xAF, 0x0407}, // CYRILLIC CAPITAL LETTER YI
		{0xB0, 0x00B0}, // DEGREE SIGN
		{0xB1, 0x00B1}, // PLUS-MINUS SIGN
		{0xB2, 0x0406}, // CYRILLIC CAPITAL LETTER BYELORUSSIAN-UKRAINIAN I
		{0xB3, 0x0456}, // CYRILLIC SMALL LETTER BYELORUSSIAN-UKRAINIAN I
		{0xB4, 0x0491}, // CYRILLIC SMALL LETTER GHE WITH UPTURN
		{0xB5, 0x00B5}, // MICRO SIGN
		{0xB6, 0x00B6}, // PILCROW SIGN
		{0xB7, 0x00B7}, // MIDDLE DOT
		{0xB8, 0x0451}, // CYRILLIC SMALL LETTER IO
		{0xB9, 0x2116}, // NUMERO SIGN
		{0xBA, 0x0454}, // CYRILLIC SMALL LETTER UKRAINIAN IE
		{0xBB, 0x00BB}, // RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK
		{0xBC, 0x0458}, // CYRILLIC SMALL LETTER JE
		{0xBD, 0x0405}, // CYRILLIC CAPITAL LETTER DZE
		{0xBE, 0x0455}, // CYRILLIC SMALL LETTER DZE
		{0xBF, 0x0457} // CYRILLIC SMALL LETTER YI
};






int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n)
{
	int i = 0;
	int j = 0;
	for (; i < (int)n && utf8[i] != 0; ++i) {
		char prefix = utf8[i];
		char suffix = utf8[i + 1];
		if ((prefix & 0x80) == 0) {
			windows1251[j] = (char)prefix;
			++j;
		}
		else if ((~prefix) & 0x20) {
			int first5bit = prefix & 0x1F;
			first5bit <<= 6;
			int sec6bit = suffix & 0x3F;
			int unicode_char = first5bit + sec6bit;


			if (unicode_char >= 0x410 && unicode_char <= 0x44F) {
				windows1251[j] = (char)(unicode_char - 0x350);
			}
			else if (unicode_char >= 0x80 && unicode_char <= 0xFF) {
				windows1251[j] = (char)(unicode_char);
			}
			else if (unicode_char >= 0x402 && unicode_char <= 0x403) {
				windows1251[j] = (char)(unicode_char - 0x382);
			}
			else {
				int count = sizeof(g_letters) / sizeof(Letter);
				for (int k = 0; k < count; ++k) {
					if (unicode_char == g_letters[k].unicode) {
						windows1251[j] = g_letters[k].win1251;
						goto NEXT_LETTER;
					}
				}
				// can't convert this char
				return 0;
			}
		NEXT_LETTER:
			++i;
			++j;
		}
		else {
			// can't convert this chars
			return 0;
		}
	}
	windows1251[j] = 0;
	return 1;
}


#define LINE_MAX 512

/*
 =================
 AL_InitDriver
 =================
 */
static qboolean AL_InitDriver (void) {
	char *deviceName = s_device->string;
	char *deviceName1251 = NULL;

	Com_Printf ("\n...Initializing OpenAL Driver\n");

	if (!deviceName[0])
		deviceName = NULL;
	

	if (alGetStringiSOFT) {
		deviceName1251 = malloc(sizeof(char) * LINE_MAX);

		if (deviceName) {
			convert_utf8_to_windows1251(deviceName, deviceName1251, LINE_MAX);
			Com_Printf("...Opening Device ("S_COLOR_GREEN"%s"S_COLOR_WHITE"): ", deviceName1251);
		}
		else
			Com_Printf("...Opening Default Device: ");
	}
	else {

		if (deviceName)
			Com_Printf("...Opening Device ("S_COLOR_GREEN"%s"S_COLOR_WHITE"): ", deviceName);
		else
			Com_Printf("...Opening Default Device: ");
	}
	// Open the device
	if ((alConfig.hDevice = alcOpenDevice (deviceName)) == NULL) {
		Com_Printf (S_COLOR_RED"failed\n");
		return qfalse;
	}

	if (!deviceName)
		Com_Printf ("succeeded ("S_COLOR_GREEN"%s"S_COLOR_WHITE")\n",
		alcGetString (alConfig.hDevice, ALC_DEVICE_SPECIFIER));
	else
		Com_Printf (S_COLOR_GREEN"succeeded\n");

	// Create the AL context and make it current
	Com_Printf ("...Creating AL Context: ");

	qboolean hrtf = qfalse;
	if (alcIsExtensionPresent(alConfig.hDevice, "ALC_SOFT_HRTF") == AL_TRUE) {
		hrtf = qtrue;
	}

	int quality;
	#ifdef _WIN32
		quality = 48000;
	#else
		quality = 44100; //wtf? soft al under linux use only 44100hz
	#endif

		ALCint attrlist[6] = 
		{	ALC_FREQUENCY, quality, 
			ALC_HRTF_SOFT, s_useHRTF->integer && hrtf ? ALC_TRUE : AL_FALSE,
		0 };
		
		if ((alConfig.hALC =
			alcCreateContext (alConfig.hDevice, attrlist)) == NULL) {
			Com_Printf (S_COLOR_RED"failed\n");
			goto failed;
		}
	
	Com_Printf (S_COLOR_GREEN"succeeded\n");

	Com_Printf ("...Making Context Current: ");
	if (!alcMakeContextCurrent (alConfig.hALC)) {
		Com_Printf (S_COLOR_RED"failed\n");
		goto failed;
	}
	Com_Printf (S_COLOR_GREEN"succeeded\n");

	Com_Printf("\n=====================================\n\n");

	if (hrtf) {
		Com_Printf("...using ALC_SOFT_HRTF\n");
		ALCint	hrtfState;
		alcGetIntegerv(alConfig.hDevice, ALC_HRTF_SOFT, 1, &hrtfState);
		if (!hrtfState)
			Com_Printf("...HRTF Mode:" S_COLOR_YELLOW " off\n");
		else
		{
			const ALchar *name = alcGetString(alConfig.hDevice, ALC_HRTF_SPECIFIER_SOFT);
			Com_Printf("...using " S_COLOR_GREEN "%s\n", name);
			Com_Printf("...HRTF Mode:" S_COLOR_GREEN " on\n");
		}

	}else
		Com_Printf(S_COLOR_MAGENTA"...ALC_SOFT_HRTF not found\n");

	return qtrue;

failed:

	Com_Printf (S_COLOR_RED"...failed hard\n");

	openalStop = qtrue;

	if (alConfig.hALC) {
		alcDestroyContext (alConfig.hALC);
		alConfig.hALC = NULL;
	}

	if (alConfig.hDevice) {
		alcCloseDevice (alConfig.hDevice);
		alConfig.hDevice = NULL;
	}

	return qfalse;
}


/*
 =================
 AL_StartOpenAL
 =================
 */

qboolean AL_StartOpenAL (void) {
	extern const char *al_device[];
	char* playback1251[7], *playback1251_def = NULL;
	int j;

	// Get device list
	if (alcIsExtensionPresent (NULL, "ALC_ENUMERATE_ALL_EXT")) { // find all ))
		unsigned i = 0;
		const ALCchar *a = alcGetString (NULL, ALC_ALL_DEVICES_SPECIFIER);
		const ALCchar *b = alcGetString (NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);

		if (alGetStringiSOFT) {
			playback1251_def = malloc(sizeof(char) * LINE_MAX);
			convert_utf8_to_windows1251(b, playback1251_def, LINE_MAX);
		}

		if (!a) {
			// We have no audio output devices. No hope.
			QAL_Shutdown ();
			return qfalse;
		}
		
		Com_Printf("======" S_COLOR_YELLOW " Available Output Devices " S_COLOR_WHITE "=====\n\n");
		

		while (*a) { // fill oal device lis
			al_device[++i] = a;
			if (!alGetStringiSOFT) {
				Com_Printf(">:" S_COLOR_GREEN " %s\n", a);
			}
			else {
				playback1251[i] = malloc(sizeof(char) * LINE_MAX);
				convert_utf8_to_windows1251((const char*)al_device[i], playback1251[i], LINE_MAX);
				Com_Printf(">:" S_COLOR_GREEN " %s\n", playback1251[i]);
			}
			while (*a)
					a++;
			a++;
		}

		alConfig.device_count = i;
	/*	if (alGetStringiSOFT) {
			for (j = 0; j <= alConfig.device_count; j++){
					playback1251[j] = malloc(sizeof(char) * LINE_MAX);
					convert_utf8_to_windows1251((const char*)al_device[j], playback1251[j], LINE_MAX);
					Com_Printf(">:" S_COLOR_GREEN " %s\n", playback1251[j]);			
				}
			}	
*/
			if (alGetStringiSOFT)
				Com_Printf("Default Output Device: " S_COLOR_GREEN "%s\n", playback1251_def);
			else
				Com_Printf("Default Output Device: " S_COLOR_GREEN "%s\n", b);
			
		//	if(alGetStringiSOFT)
		//		for (j = 0; j <= alConfig.device_count; j++)
		//			free(playback1251[j]);
					
			if(playback1251_def)
				free(playback1251_def);

			
	}
	else {		

		QAL_Shutdown ();
		return qfalse;
	}

	Com_Printf("\n=====================================\n");

	// Initialize the device, context, etc...
	if (AL_InitDriver ()) {
		return qtrue;
	}
	else {
		QAL_Shutdown ();
		return qfalse;
	}
}

/*
 =================
 AL_Shutdown
 =================
 */
void AL_Shutdown (void) {
	Com_Printf ("Shutting down OpenAL subsystem\n");

	if (alConfig.hALC) {
		if (alcMakeContextCurrent) {
			Com_Printf ("...alcMakeContextCurrent( NULL ): ");
			if (!alcMakeContextCurrent (NULL))
				Com_Printf ("failed\n");
			else
				Com_Printf ("succeeded\n");
		}

		if (alcDestroyContext) {
			Com_Printf ("...destroying AL context\n");
			alcDestroyContext (alConfig.hALC);
		}

		alConfig.hALC = NULL;
	}

	if (alConfig.hDevice) {
		if (alcCloseDevice) {
			Com_Printf ("...closing device\n");
			alcCloseDevice (alConfig.hDevice);
		}

		alConfig.hDevice = NULL;
	}

	QAL_Shutdown ();
}
