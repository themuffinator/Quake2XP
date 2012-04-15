#include "client.h"
#include "cdaudio.h"
#include "snd_loc.h"

qboolean LoadWAV2(char *name, void **wav, int *outBits, int *outChannels, ALsizei * rate, ALsizei * size);

typedef enum {
	MSTAT_STOPPED, MSTAT_PAUSED, MSTAT_PLAYING
} mstat_t;

static mstat_t mstat;

static music_type_t music_type;

//byte music_buffer[MAX_STRBUF_SIZE];
byte music_buffer[0x1000];

void *mFile;
int mFilePos, mFileSize;

void Music_Init(void) {
	music_type = s_musicsrc->value;
	mstat = MSTAT_STOPPED;

	Com_Printf("Music init\n");

	switch (music_type) {
		case MUSIC_NONE:
			return;
		case MUSIC_CD:
			CDAudio_Init();
			break;
		case MUSIC_FILES:
			break;
		default:
			Cvar_SetValue("s_musicsrc", MUSIC_NONE);
			music_type = MUSIC_NONE;
			return;
	}
}

void Music_Shutdown(void) {
	Music_Stop();

	Com_Printf("Music shutdown\n");

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Shutdown();
			break;
		case MUSIC_FILES:
			break;
	}
}

void Music_Play(void) {
	int track = atoi(cl.configstrings[CS_CDTRACK]);
	char name[MAX_QPATH];
	int format, rate, bits, channels;

	Music_Stop();

	// TODO: support random track/random file, unify with cd_* variables which are general enough
#if 0
	if (track == 0) {
		return;
	}
#endif

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Play(track, true);
			mstat = MSTAT_PLAYING;
			break;
		case MUSIC_FILES:
			Q_snprintfz(name, sizeof(name), "music/track%02i.wav", track);
			//Q_snprintfz(name, sizeof(name), "music/test.wav");
			if (LoadWAV2(name, &mFile, &bits, &channels, &rate, &mFileSize)) {
				mstat = MSTAT_PLAYING;
				mFilePos = 0;
				S_Streaming_Start(bits, channels, rate, s_musicvolume->value);
			} else
				Com_Printf("Music_Play: unable to load music from %s\n");
			break;
	}
}

void Music_Stop(void) {
	if (mstat == MSTAT_STOPPED)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Stop();
			break;
		case MUSIC_FILES:
			S_Streaming_Stop();
			free(mFile);
			break;
	}

	mstat == MSTAT_STOPPED;
}

void Music_Pause() {
	if (mstat != MSTAT_PLAYING)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Activate(false);
			break;
		case MUSIC_FILES:
			alSourcePause(source_name[CH_STREAMING]);
			break;
	}

	mstat = MSTAT_PAUSED;
}

void Music_Resume() {
	if (mstat != MSTAT_PAUSED)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Activate(false);
			break;
		case MUSIC_FILES:
			alSourcePlay(source_name[CH_STREAMING]);
			break;
	}

	mstat = MSTAT_PLAYING;
}


void Music_Update(void) {
	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Update();
			break;
		case MUSIC_FILES:
			if (mstat == MSTAT_PLAYING) {
				int n;

				// TODO: now similar code can be removed from menu
				if (s_musicvolume->modified) {
					alSourcef(source_name[CH_STREAMING], AL_GAIN, s_musicvolume->value);
					s_musicvolume->modified = false;
				}
				// FIXME: do something more intelligent, like returning the free space in S_Streaming_Add or sleeping for a number of frames
				n = S_Streaming_Add(mFile + mFilePos, 1024*64);
				//Com_Printf("Inserted %d bytes of music, pos %d\n", n, mFilePos);
				mFilePos += n;
				if (mFilePos == mFileSize)
					// play something else
					Music_Play();
			}
			break;
	}
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
