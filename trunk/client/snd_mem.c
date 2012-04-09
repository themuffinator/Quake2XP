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
// snd_mem.c: sound caching

#include "client.h"
#include "snd_loc.h"

#define PCM_16BIT	1			// willow: do not touch this!!
#define PCM_STEREO	2			// willow: do not touch this!!

byte *S_Alloc(int size);


//=============================================================================

static qboolean LoadWAV(char *name, byte ** wav, ALenum * format,
						ALsizei * rate, ALsizei * size);

long stream_info_samples_initial;
long stream_info_dataofs_initial;



// =======================================================================
// Load a sound
// =======================================================================
/*
==================
S_FindName
==================
*/
ALuint S_FindName(char *name, qboolean create)
{
	int i;
	char real_name[MAX_QPATH];
	byte *data;
	ALsizei rate;
	ALenum format;
	ALsizei size;
	extern int num_sfx;
	extern char known_sfx_name[MAX_SFX][MAX_QPATH];
	extern ALuint known_sfx_bufferNum[MAX_SFX];
	extern char *fastsound_name[FAST_SOUNDS_TOTAL];

	if(!s_initsound->value || openalStop)
		return 0;

	if (!name)
		Com_Error(ERR_FATAL, "S_FindName: NULL\n");
	if (!name[0])
		Com_Error(ERR_FATAL, "S_FindName: empty name\n");

	if (strlen(name) >= MAX_QPATH)
		Com_Error(ERR_FATAL, "Sound name too long: %s", name);

	// check internal fastsound effects first
	for (i = 0; i < FAST_SOUNDS_TOTAL; i++)
		if (!strcmp(fastsound_name[i], name)) {
			Com_DPrintf("Legacy code requested fastsound sfx caching!\n");
			Com_DPrintf("cache request ignored: %s\n", name);
			return fastsound_descriptor[i];
		}
	// see if already loaded
	for (i = 0; i < num_sfx; i++)
		if (!strcmp(known_sfx_name[i], name)) {
			return known_sfx_bufferNum[i];
		}

	if (!create)
		return 0;

	// find a free sfx
	for (i = 0; i < num_sfx; i++)
		if (!known_sfx_name[i][0])
			break;

	// willow: TO DO smthg, it's really dumb solution to terminate game
	// just because out of descriptors pool.
	// Just to adjust this pool is not good solution too.
	if (num_sfx >= MAX_SFX)
		Com_Error(ERR_FATAL, "S_FindName: out of sfx_t");

	memset(known_sfx_name[i], 0, MAX_QPATH);
	strcpy(known_sfx_name[i], name);

	// Load it from disk
	if (name[0] == '#')
		Com_sprintf(real_name, sizeof(real_name), "%s", &name[1]);
	else
		Com_sprintf(real_name, sizeof(real_name), "sound/%s", name);

	if (!LoadWAV(real_name, &data, &format, &rate, &size)) {
		Com_DPrintf("BAD WAVE '%s'\n", name);
		return 0;
	} else {
		// Upload the sound
		alGenBuffers(1, &known_sfx_bufferNum[i]);
#ifdef _WITH_EAX
		if (!eaxSetBufferMode
			(1, &known_sfx_bufferNum[i],
			 alGetEnumValue("AL_STORAGE_HARDWARE"))) {
			// "AL_STORAGE_AUTOMATIC" "AL_STORAGE_HARDWARE"
			// "AL_STORAGE_ACCESSIBLE"
			Com_DPrintf("LoadSound: unable to set X-RAM mode\n");
		}
#endif
		if (alGetError() != AL_NO_ERROR) {
			Com_DPrintf("LoadSound: unable to set X-RAM mode\n");
		}
		alBufferData(known_sfx_bufferNum[i], format, data, size, rate	// the
																		// frequency
																		// of
																		// the
																		// audio
																		// data
			);
		if (alGetError() != AL_NO_ERROR) {
			Com_DPrintf("LoadSound: bad sample\n");
		}

		Z_Free(data);

		num_sfx++;
		return known_sfx_bufferNum[i];
	}
}

void S_fastsound_get_descriptors_pool(unsigned count,
									  ALuint * descriptors_pool)
{
	alGenBuffers(count, descriptors_pool);

	// "AL_STORAGE_AUTOMATIC" "AL_STORAGE_HARDWARE"
	// "AL_STORAGE_ACCESSIBLE"
#ifdef _WITH_EAX
	if (!eaxSetBufferMode
		(count, descriptors_pool, alGetEnumValue("AL_STORAGE_HARDWARE")))
		Com_DPrintf("S_fastsound: unable to set X-RAM mode\n");
#endif

	if (alGetError() != AL_NO_ERROR)
		Com_DPrintf("S_fastsound: unable to set X-RAM mode\n");
}

void S_fastsound_kill_descriptors_pool(unsigned count,
									   ALuint * descriptors_pool)
{
	alDeleteBuffers(count, descriptors_pool);
	memset(descriptors_pool, 0, count * sizeof(ALuint));
}

void S_fastsound_load(char *input_name, ALuint bufferNum)
{
	char name[MAX_QPATH];
	byte *data;
	ALsizei rate;				// the frequency of the audio data
	ALenum format;
	ALsizei size;

	// Load it from disk
	if (input_name[0] == '#')
		Com_sprintf(name, sizeof(name), "%s", &input_name[1]);
	else
		Com_sprintf(name, sizeof(name), "sound/%s", input_name);

	if (!LoadWAV(name, &data, &format, &rate, &size)) {
		Com_DPrintf("BAD WAVE '%s'\n", name);
		return;
	}
	// Upload the sound
	alBufferData(bufferNum, format, data, size, rate);
	if (alGetError() != AL_NO_ERROR) {
		Com_DPrintf("LoadSound: bad sample\n");
	}

	Z_Free(data);
}

/*
===============================================================================

WAV loading

===============================================================================
*/


byte *data_p;
byte *iff_end;
byte *last_chunk;
byte *iff_data;
int iff_chunk_len;


short GetLittleShort(void)
{
#if 0
	short val = *data_p;
	val = val + (*(data_p + 1) << 8);
#else
	short val = *((short *) data_p);
#endif
	data_p += 2;
	return val;
}

long GetLittleLong(void)
{
#if 0
	long val = *data_p;
	val = val + (*(data_p + 1) << 8);
	val = val + (*(data_p + 2) << 16);
	val = val + (*(data_p + 3) << 24);
#else
	long val = *((long *) data_p);
#endif
	data_p += 4;
	return val;
}

void FindNextChunk(char *name)
{
	while (1) {
		data_p = last_chunk;

		if (data_p >= iff_end) {	// didn't find the chunk
			data_p = NULL;
			return;
		}

		data_p += 4;
		iff_chunk_len = GetLittleLong();
		if (iff_chunk_len < 0) {
			data_p = NULL;
			return;
		}
//      if (iff_chunk_len > 1024*1024)
//          Sys_Error ("FindNextChunk: %i length is past the 1 meg sanity limit", iff_chunk_len);
		data_p -= 8;
		last_chunk = data_p + 8 + ((iff_chunk_len + 1) & ~1);
		if (!strncmp(data_p, name, 4))
			return;
	}
}

void FindChunk(char *name)
{
	last_chunk = iff_data;
	FindNextChunk(name);
}

/*
void DumpChunks(void)
{
	char	str[5];

	str[4] = 0;
	data_p=iff_data;
	do
	{
		memcpy (str, data_p, 4);
		data_p += 4;
		iff_chunk_len = GetLittleLong();
		Com_Printf ("0x%x : %s (%d)\n", (int)(data_p - 4), str, iff_chunk_len);
		data_p += (iff_chunk_len + 1) & ~1;
	} while (data_p < iff_end);
}
*/


static qboolean LoadWAV(char *name, byte ** wav, ALenum * format,
						ALsizei * rate, ALsizei * size)
{
	byte *buffer, *out;
	short channels, width;
	int length = FS_LoadFile(name, (void **) &buffer);

	if (!buffer)
		return false;

	iff_data = buffer;
	iff_end = buffer + length;

	// Find "RIFF" chunk
	FindChunk("RIFF");
	if (!(data_p && !memcmp((void *) (data_p + 8), "WAVE", 4))) {
		Com_DPrintf("S_LoadWAV: missing 'RIFF/WAVE' chunks (%s)\n", name);
		FS_FreeFile(buffer);
		return false;
	}
	// Get "fmt " chunk
	iff_data = data_p + 12;

	FindChunk("fmt ");
	if (!data_p) {
		Com_DPrintf("S_LoadWAV: missing 'fmt ' chunk (%s)\n", name);
		FS_FreeFile(buffer);
		return false;
	}

	data_p += 8;

	if (GetLittleShort() != 1) {
		Com_DPrintf("S_LoadWAV: Microsoft PCM format only (%s)\n", name);
		FS_FreeFile(buffer);
		return false;
	}

	*format = AL_FORMAT_MONO8;
	channels = GetLittleShort();
	if (channels < 1 || channels > 2) {
		Com_DPrintf
			("S_LoadWAV: only mono and stereo WAV files supported (%s)\n",
			 name);
		FS_FreeFile(buffer);
		return false;
	}
	if (channels == 2)
		*format |= PCM_STEREO;

	*rate = GetLittleLong();

	data_p += 4 + 2;

	width = GetLittleShort() >> 3;
	if (width != 1 && width != 2) {
		Com_DPrintf
			("S_LoadWAV: only 8 and 16 bit WAV files supported (%s)\n",
			 name);
		FS_FreeFile(buffer);
		return false;
	}
	if (width == 2)
		*format |= PCM_16BIT;

	// Find data chunk
	FindChunk("data");
	if (!data_p) {
		Com_DPrintf("S_LoadWAV: missing 'data' chunk (%s)\n", name);
		FS_FreeFile(buffer);
		return false;
	}

	data_p += 4;
	*size = GetLittleLong();

	if (*size == 0) {
		Com_DPrintf("S_LoadWAV: file with 0 samples (%s)\n", name);
		FS_FreeFile(buffer);
		return false;
	}
	// Load the data
	*wav = out = Z_TagMalloc(*size, TAGMALLOC_CLIENT_SOUNDCACHE);
	memcpy(out, buffer + (data_p - buffer), *size);

	FS_FreeFile(buffer);

	return true;
}





byte *stream_wav;
long stream_info_rate;
long stream_info_samples;
long stream_info_dataofs;

void StreamingWav_close(void)
{
	FS_FreeFile(stream_wav);
}

/*
============
wave file streaming service
============
*/
qboolean StreamingWav_init(char *name)
{
	int samples;

	int wavlength = FS_LoadFile(name, (void **) &stream_wav);

	if (!stream_wav)
		return 0;

	iff_data = stream_wav;
	iff_end = stream_wav + wavlength;

// find "RIFF" chunk
	FindChunk("RIFF");
	if (!(data_p && !strncmp(data_p + 8, "WAVE", 4))) {
		Com_Printf("Missing RIFF/WAVE chunks\n");
		StreamingWav_close();
		return 0;
	}
// get "fmt " chunk
	iff_data = data_p + 12;
// DumpChunks ();

	FindChunk("fmt ");
	if (!data_p) {
		Com_Printf("Missing fmt chunk\n");
		StreamingWav_close();
		return 0;
	}
	data_p += 8;
	if (GetLittleShort() != 1) {
		Com_Printf("Microsoft PCM format only\n");
		StreamingWav_close();
		return 0;
	}
	if (GetLittleShort() != 2) {
		Com_Printf("Stereo files only\n");
		StreamingWav_close();
		return 0;
	}

	stream_info_rate = GetLittleLong();
	data_p += 4 + 2;

	if (GetLittleShort() != 16) {
		Com_Printf("16 bit files only\n");
		StreamingWav_close();
		return 0;
	}
// get cue chunk
/*	FindChunk("cue ");
	if (data_p)
	{
		int			i;
		data_p += 32;
		info.loopstart = GetLittleLong();
//		Com_Printf("loopstart=%d\n", sfx->loopstart);

	// if the next chunk is a LIST chunk, look for a cue length marker
		FindNextChunk ("LIST");
		if (data_p)
		{
			if (!strncmp (data_p + 28, "mark", 4))
			{	// this is not a proper parse, but it works with cooledit...
				data_p += 24;
				i = GetLittleLong ();	// samples in loop
				info.samples = info.loopstart + i;
//				Com_Printf("looped length: %i\n", i);
			}
		}
	}
	else
		info.loopstart = -1;*/

// find data chunk
	FindChunk("data");
	if (!data_p) {
		Com_Printf("Missing data chunk\n");
		StreamingWav_close();
		return 0;
	}

	data_p += 4;
	samples = GetLittleLong() >> 1;
/*
	if (stream_info.samples)
	{
		if (samples < stream_info.samples)
			Com_Error (ERR_DROP, "Sound %s has a bad loop length", name);
	}
	else*/
	stream_info_samples = samples;

	stream_info_dataofs = data_p - stream_wav;
//  memcpy(out, buffer + (data_p - buffer), *size);

  stream_info_samples = stream_info_samples_initial = samples;
  stream_info_dataofs = stream_info_dataofs_initial = data_p - stream_wav;

	return 1;					// info;
}

void StreamingWav_Reset(void)
{
     stream_info_samples = stream_info_samples_initial;
     stream_info_dataofs = stream_info_dataofs_initial;
}
