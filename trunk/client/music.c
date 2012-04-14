#include "client.h"
#include "cdaudio.h"
#include "snd_loc.h"

static music_type_t music_type;

static qboolean playing = false;

void S_Play_Wav_Music(void)
{
	char	name[MAX_QPATH];
	int		track;
	
	if (s_musicsrc->value == MUSIC_NONE){
		CDAudio_Stop();
		S_Streaming_Stop();	
		return;
	}
	//}
	track = atoi(cl.configstrings[CS_CDTRACK]);

	if (track == 0){
		// Stop any playing track
		CDAudio_Stop();
		S_Streaming_Stop();
		return;
	}

	// If an wav file exists play it, otherwise fall back to CD audio
	Q_snprintfz(name, sizeof(name), "music/track%02i.wav", track);
	if (FS_LoadFile(name, NULL) != -1)
		//S_StartBackgroundTrack(name, name);
		;
	else
		CDAudio_Play(track, true);

}

void Music_Init(void) {
	if (music_type != -1)
		return;

	music_type = s_musicsrc->value;
	switch (music_type) {
		case MUSIC_NONE:
		case MUSIC_CD:
		case MUSIC_FILES:
		default:
			break;
	}
}

void Music_Shutdown(void) {
	if (s_musicsrc->value == MUSIC_CD)
		CDAudio_Shutdown();
}

void Music_Play(void) {
}

void Music_Stop(void) {
#if 0
	if (streaming == 0) {
		return;
	} else if (streaming != 1) {
		Com_Printf(S_COLOR_RED"Music_Stop(): streaming is %d; should be 0 or 2 here\n");
		return;
	}
#endif

	//S_Streaming_Stop();
	
	// choice dependent
	//CDAudio_Stop();
	//StreamingWav_close();
}

void Music_Update(void) {
}

// TODO: rewrite with FS_ListFiles and similar, like snd_ogg.c
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

		//S_StartBackgroundTrack(intro, loop);
	} else
		//S_StartBackgroundTrack(intro, intro);
		;
}


#if 0
void S_UpdateBackgroundTrack()
{
	ALint iState;
	ALenum format;
	int iBuffersProcessed = 0;

	if (!streaming)
		return;

	// Request the number of OpenAL Buffers have been processed
	// (played) on the Source
	alGetSourcei(source_name[CH_STREAMING], AL_BUFFERS_PROCESSED, &iBuffersProcessed);

	// For each processed buffer, remove it from the Source Queue,
	// read next chunk of audio
	// data from disk, fill buffer with new data, and add it to the
	// Source Queue
	while (iBuffersProcessed) {
		// Remove the Buffer from the Queue.  (uiBuffer contains the
		// Buffer ID for the unqueued Buffer)
		ALuint uiBuffer = 0;
		alSourceUnqueueBuffers(source_name[CH_STREAMING], 1,
								&uiBuffer);

		begin:
		ulBytesWritten = (ulBufferSize > stream_info_samples * 2) ? stream_info_samples * 2 : ulBufferSize;
		if (ulBytesWritten) {
				memcpy(pData, stream_wav + stream_info_dataofs,
						ulBytesWritten);
				{
					alBufferData(uiBuffer, AL_FORMAT_STEREO16, pData, ulBytesWritten, stream_info_rate);
					alSourceQueueBuffers(source_name [CH_STREAMING], 1, &uiBuffer);
				}
				stream_info_samples -= ulBytesWritten / 2;
				stream_info_dataofs += ulBytesWritten;
		}
		else
		{
				if (BackgroundTrack_Repeat)
					StreamingWav_Reset();
				goto begin;
		}

		iBuffersProcessed--;
	}

	// Check the status of the Source.  If it is not playing, then
	// playback was completed,
	// or the Source was starved of audio data, and needs to be
	// restarted.


	alGetSourcei(source_name[CH_STREAMING], AL_SOURCE_STATE,
					&iState);
		if (iState != AL_PLAYING) {
			ALint iQueuedBuffers;
			// If there are Buffers in the Source Queue then the Source
			// was starved of audio
			// data, so needs to be restarted (because there is more audio
			// data to play)
			alGetSourcei(source_name[CH_STREAMING], AL_BUFFERS_QUEUED, &iQueuedBuffers);
			if (iQueuedBuffers) {
				alSourcePlay(source_name[CH_STREAMING]);
			} else {
				// Finished playing
				S_Streaming_Stop();
			}
	}
}

void S_StartBackgroundTrack(char *introTrack, char *loopTrack)
{
	 BackgroundTrack_Repeat = loopTrack != 0;
	
	if (streaming)
		S_Streaming_Stop();

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
					alSourceQueueBuffers(source_name[CH_STREAMING],
										 1, &uiBuffers[i]);
				}
				stream_info_samples -= ulBytesWritten / 2;
				stream_info_dataofs += ulBytesWritten;
			}
		}
		// Start playing source
		alSourcef(source_name[CH_STREAMING], AL_GAIN, s_musicvolume->value);
		alSourcePlay(source_name[CH_STREAMING]);
		streaming = true;

	} else
		Com_Printf("Failed to load %s\n", introTrack);
}
#endif
