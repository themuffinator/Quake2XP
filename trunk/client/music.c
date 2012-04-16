#include "client.h"
#include "cdaudio.h"
#include "snd_loc.h"

// Implementation specific header, do not include outside music.c
#include "music.h"

typedef enum {
	MSTAT_STOPPED, MSTAT_PAUSED, MSTAT_PLAYING
} mstat_t;

static mstat_t mstat = MSTAT_STOPPED;
static music_type_t music_type = MUSIC_NONE;
static Gen_Interface_t *music_handle;
static byte music_buffer[MAX_STRBUF_SIZE];

static char **fsList;
static int fsIndex;
static int fsNumFiles;
static int fsNameOffset;

#if 0
#define MAX_PATTERNS 16
char **FS_ListFilesMany(int numPatterns, char **patterns, int *nfiles, int musthave, int canthave) {
	int i, j, ntotal, pos;
	int subNumFiles[MAX_PATTERNS];
	char **subNameLists[MAX_PATTERNS];
	char **res;

	ntotal = 0;
	for (i = 0, j = 0; i < numPatterns; i++) {
		subNameLists[j] = FS_ListFiles(patterns[j], &subNumFiles[j], musthave, canthave);
		subNumFiles[i]--;
		if (subNameLists[j] != NULL) {
			ntotal += subNumFiles[j];
			j++;
		}
	}
	if (j == 0) {
		*nfiles = 0;
		return NULL;
	}

	res = malloc((ntotal+1) * sizeof(char*));
	pos = 0;
	for (i = 0; i < j; i++) {
		memcpy(res + pos, subNameLists[i], sizeof(char*) * (subNumFiles[i]));
		free(subNameLists[i]);
		pos += subNumFiles[i];
	}
	pos++;
	assert(pos == ntotal);
	return res;
}
#endif

void Music_Init(void) {
	char path[MAX_QPATH];

	music_type = s_musicsrc->value;
	mstat = MSTAT_STOPPED;

	Com_Printf("\n========Init Music subsistem========\n\n");

	switch (music_type) {
		case MUSIC_NONE:
			return;
		case MUSIC_CD:
			CDAudio_Init();
			break;
		case MUSIC_CD_FILES:
			break;
		case MUSIC_OTHER_FILES:
			Q_snprintfz(path, sizeof(path), "%s/music/*", FS_Gamedir());
			Com_DPrintf(S_COLOR_MAGENTA "Music_Init: searching files in \"%s\"\n", path);
			fsList = FS_ListFiles(path, &fsNumFiles, 0, SFF_SUBDIR);
			fsNumFiles--;
			fsIndex = -1;
			fsNameOffset = strlen(FS_Gamedir())+1;

			if (fsList != NULL)
				Com_Printf(S_COLOR_YELLOW "found "S_COLOR_GREEN"%d "S_COLOR_YELLOW"music files\n\n", fsNumFiles);
			Com_Printf("====================================\n\n");
			break;
		default:
			Cvar_SetValue("s_musicsrc", MUSIC_NONE);
			music_type = MUSIC_NONE;
			return;
	}
}

void Music_Shutdown(void) {
	if (music_type == MUSIC_NONE)
		return;

	Com_Printf("Music shutdown\n");
	Music_Stop();

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Shutdown();
			break;
		case MUSIC_CD_FILES:
			break;
		case MUSIC_OTHER_FILES:
			fsNumFiles++;
			FS_FreeList(fsList, fsNumFiles);
			break;
	}
	music_type = MUSIC_NONE;
}
// only to be called inside Music_Play
static qboolean Music_PlayFile(const char *name, qboolean hasExt) {
	soundparams_t sp;

	if (hasExt)
		music_handle = Gen_Open(name, &sp);
	else
		music_handle = Gen_OpenAny(name, &sp);

	if (music_handle != NULL) {
		if (hasExt)
			Com_DPrintf(S_COLOR_GREEN "Music_Play: playing \"%s\"\n", name);
		else
			Com_DPrintf(S_COLOR_GREEN "Music_Play: playing \"%s.%s\"\n", name, music_handle->ext);

		S_Streaming_Start(sp.bits, sp.channels, sp.rate, s_musicvolume->value);
		mstat = MSTAT_PLAYING;
		return true;
	} else {
		Com_Printf(S_COLOR_YELLOW "Music_Play: unable to load \"%s\"\n", name);
		return false;
	}
}

void Music_Play(void) {
	int track, count;
	char name[MAX_QPATH];

	Music_Stop();

	if (music_type != MUSIC_OTHER_FILES &&
	  track == 0 && s_musicrandom->value == 0)
		return;

	if (s_musicrandom->value != 0)
		// original soundtrack has tracks 2 to 11
		track = 2 + rand()%10;
	else
		track = atoi(cl.configstrings[CS_CDTRACK]);

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Play(track, true);
			mstat = MSTAT_PLAYING;
			break;

		case MUSIC_CD_FILES:
			Q_snprintfz(name, sizeof(name), "music/%02i", track);
			Music_PlayFile(name, false);
			break;

		case MUSIC_OTHER_FILES:
			if (fsList == NULL)
				return;

			if (s_musicrandom->value != 0)
				fsIndex = rand() % fsNumFiles;
			else
				fsIndex = (fsIndex+1) % fsNumFiles;
			count = fsNumFiles;
			while (count-- > 0) {
				if (Music_PlayFile(fsList[fsIndex] + fsNameOffset, true))
					return;
				fsIndex = (fsIndex+1) % fsNumFiles;
			}
			break;
	}
}

void Music_Stop(void) {
	if (mstat == MSTAT_STOPPED)
		return;

	Com_DPrintf(S_COLOR_GREEN "Stopped playing music\n");

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Stop();
			break;
		case MUSIC_CD_FILES:
		case MUSIC_OTHER_FILES:
			music_handle->close(music_handle->f);
			S_Streaming_Stop();
			break;
	}

	mstat = MSTAT_STOPPED;
}

void Music_Pause(void) {
	if (mstat != MSTAT_PLAYING)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Activate(false);
			break;
		case MUSIC_CD_FILES:
		case MUSIC_OTHER_FILES:
			alSourcePause(source_name[CH_STREAMING]);
			break;
	}

	mstat = MSTAT_PAUSED;
}

void Music_Resume(void) {
	if (mstat != MSTAT_PAUSED)
		return;

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Activate(true);
			break;
		case MUSIC_CD_FILES:
		case MUSIC_OTHER_FILES:
			alSourcePlay(source_name[CH_STREAMING]);
			break;
	}

	mstat = MSTAT_PLAYING;
}

void Music_Update(void) {
	int n;

	// if we are in the configuration menu, or paused we don't do anything
	if (mstat == MSTAT_PAUSED)
		return;

	// Check for configuration changes
	
	if (s_musicsrc->modified) {
		Music_Shutdown();
		Music_Init();
		Music_Play();
		s_musicsrc->modified = false;
		s_musicvolume->modified = false;
		s_musicrandom->modified = false;
		return;
	}

	if (music_type == MUSIC_NONE)
		return;

	if (s_musicrandom->modified) {
		s_musicrandom->modified = false;
		Music_Play();
		return;
	}

	if (s_musicvolume->modified) {
		switch (music_type) {
			case MUSIC_CD:
				Cvar_SetValue("cd_volume", s_musicvolume->value);
				break;
			case MUSIC_CD_FILES:
			case MUSIC_OTHER_FILES:
				alSourcef(source_name[CH_STREAMING], AL_GAIN, s_musicvolume->value);
				break;
		}
		s_musicvolume->modified = false;
		return;
	}

	// Do the actual update

	switch (music_type) {
		case MUSIC_CD:
			CDAudio_Update();
			break;

		case MUSIC_CD_FILES:
		case MUSIC_OTHER_FILES:
			if (mstat != MSTAT_PLAYING || S_Streaming_NumFreeBufs() == 0)
				return;

			// Play a portion of the current file
			n = music_handle->read(music_handle->f, music_buffer, sizeof(music_buffer));
			if (n == 0) {
				Music_Play();
			} else {
				// don't check return value as the buffer is guaranteed to fit
				S_Streaming_Add(music_buffer, n);
			}
			break;
	}
}

void S_Music_f(void)
{
	// CD: uses the level track, or another one if s_musicrandom
	// Other: advances to the next, or random if s_musircandom
	Music_Play();
}

// Generic interface and type specific implementations

supported_exts_t supported_exts[] = {
	{"wav", (openFunc_t)MC_OpenWAV},
	{"ogg", (openFunc_t)MC_OpenVorbis}
};

// Gen_OpenAny: try all possible extensions of filename in sequence
static Gen_Interface_t *Gen_OpenAny(const char *name, soundparams_t *sp) {
	// check all supported extensions, or better try in a loop somewhere else?
	int i;

	for (i = 0; i < sizeof(supported_exts)/sizeof(supported_exts[0]); i++) {
		char path[MAX_QPATH];
		Gen_Interface_t *res;

		Q_snprintfz(path, sizeof(path), "%s.%s", name, supported_exts[i].name);
		res = supported_exts[i].openFunc(path, sp);
		if (res != NULL)
			return res;
	}
	return NULL;
}

// Gen_Open: check given filename and call appropiate routine
static Gen_Interface_t *Gen_Open(const char *name, soundparams_t *sp) {
	int i;
	const char *ext = strrchr(name, '.');

	if (ext == NULL)
		return NULL;

	Com_DPrintf("trying to load %s\n", name);

	for (i = 0; i < sizeof(supported_exts)/sizeof(supported_exts[0]); i++)
		if (Q_strcasecmp(ext + 1, supported_exts[i].name) == 0)
			return supported_exts[i].openFunc(name, sp);

	return NULL;
}

static int MC_ReadVorbis(MC_Vorbis_t *f, char *buffer, int n) {
	int total = 0;
	const int step = 1024*64;

	assert(step < n);
	while (total + step < n) {
		// FIXME: check endianess
		int cur = ov_read(&f->ovFile, buffer + total, step, 0, 2, 1, &f->pos);
		if (cur < 0)
			return 0;
		if (cur == 0)
			return total;
		total += cur;
	}
	return total;
}

static void MC_RewindVorbis(MC_Vorbis_t *f) {
	f->pos = 0;
}

static void MC_CloseVorbis(MC_Vorbis_t *f) {
	ov_clear(&f->ovFile);
	FS_FreeFile(f->ovRawFile);
}

static Gen_Interface_t *MC_OpenVorbis(const char *name, soundparams_t *sp) {
	static MC_Vorbis_t f;
	static Gen_Interface_t res;

	f.size = FS_LoadFile(name, &f.ovRawFile);
	if (f.size < 0)
		return NULL;

	if (ov_open(NULL, &f.ovFile, f.ovRawFile, f.size) == 0) {
		f.info = ov_info(&f.ovFile, 0);
		f.pos = 0;
		sp->bits = 16;
		sp->channels = f.info->channels;
		sp->rate = f.info->rate;

		res.read = (readFunc_t)MC_ReadVorbis;
		res.rewind = (rewindFunc_t)MC_RewindVorbis;
		res.close = (closeFunc_t)MC_CloseVorbis;
		res.f = &f;
		res.ext = "ogg";

		return &res;
	} else {
		FS_FreeFile(f.ovRawFile);
		return NULL;
	}
}

static int MC_ReadWAV(MC_WAV_t *f, void *buffer, int n) {
	const int r = MIN(n, f->size - f->pos);

	if (n > 0) {
		memcpy(buffer, f->start + f->pos, r);
		f->pos += r;
	}

	return n;
}

static void MC_RewindWAV(MC_WAV_t *f) {
	f->pos = 0;
}

static void MC_CloseWAV(MC_WAV_t *f) {
	FS_FreeFile(f->data);
}

static Gen_Interface_t *MC_OpenWAV(const char *name, soundparams_t *sp) {
	static MC_WAV_t f;
	static Gen_Interface_t res;

	if (S_LoadWAV(name, &f.data, &f.start, &sp->bits, &sp->channels, &sp->rate, &f.size)) {
		f.pos = 0;

		res.read = (readFunc_t)MC_ReadWAV;
		res.rewind = (rewindFunc_t)MC_RewindWAV;
		res.close = (closeFunc_t)MC_CloseWAV;
		res.f = &f;
		res.ext = "wav";

		return &res;
	}
	return NULL;
}
