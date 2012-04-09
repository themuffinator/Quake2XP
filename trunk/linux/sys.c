/*
 * Copyright (C) 1997-2001 Id Software, Inc.
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
#ifdef __linux__
#include <execinfo.h>
#endif

#include "../qcommon/qcommon.h"

unsigned	sys_frame_time;

qboolean	stdin_active = true;

uid_t saved_euid;

/* ======================================================================= */
/* General routines */
/* ======================================================================= */

void
Sys_ConsoleOutput(char *string)
{
	fputs(string, stdout);
}

void
Sys_Printf(char *fmt,...)
{
	va_list		argptr;
	char		text[1024];
	unsigned char  *p;

	va_start(argptr, fmt);
	vsnprintf(text, sizeof(text), fmt, argptr);
	va_end(argptr);

	for (p = (unsigned char *)text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}

void
Sys_Quit(void)
{
	CL_Shutdown();
	Qcommon_Shutdown();
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~FNDELAY);
	exit(0);
}

void
Sys_Init(void)
{
}

void
Sys_Error(char *error,...)
{
	va_list		argptr;
	char		string[1024];

	/* change stdin to non blocking */
	fcntl(0, F_SETFL, fcntl(0, F_GETFL, 0) & ~FNDELAY);

	CL_Shutdown();
	Qcommon_Shutdown();

	va_start(argptr, error);
	vsnprintf(string, sizeof(string), error, argptr);
	va_end(argptr);
	fprintf(stderr, "Error: %s\n", string);

	exit(1);

}

void
Sys_Warn(char *warning,...)
{
	va_list		argptr;
	char		string[1024];

	va_start(argptr, warning);
	vsnprintf(string, sizeof(string), warning, argptr);
	va_end(argptr);
	fprintf(stderr, "Warning: %s", string);
}

/*
 * ============ Sys_FileTime
 *
 * returns -1 if not present ============
 */
int
Sys_FileTime(char *path)
{
	struct stat	buf;

	if (stat(path, &buf) == -1)
		return -1;

	return buf.st_mtime;
}

void
floating_point_exception_handler(int whatever)
{
	signal(SIGFPE, floating_point_exception_handler);
}

char           *
Sys_ConsoleInput(void)
{
	static char	text[256];
	int		len;
	fd_set		fdset;
	struct timeval	timeout;

	if (!dedicated || !dedicated->value)
		return NULL;

	if (!stdin_active)
		return NULL;

	FD_ZERO(&fdset);
	FD_SET(0, &fdset);	/* stdin */
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	if (select(1, &fdset, NULL, NULL, &timeout) == -1 || !FD_ISSET(0, &fdset))
		return NULL;

	len = read(0, text, sizeof(text));
	if (len == 0) {		/* eof! */
		stdin_active = false;
		return NULL;
	}
	if (len < 1)
		return NULL;
	text[len - 1] = 0;	/* rip off the /n and terminate */

	return text;
}

/*****************************************************************************/

static void    *game_library;

/*
 * ================= Sys_UnloadGame =================
 */
void
Sys_UnloadGame(void)
{
	if (game_library)
		dlclose(game_library);
	game_library = NULL;
}

extern cvar_t *net_compatibility;
/*
 * ================= Sys_GetGameAPI
 *
 * Loads the game dll =================
 */
void           *
Sys_GetGameAPI(void *parms)
{
	void           *(*GetGameAPI) (void *);
	FILE           *fp;
	char		name[MAX_OSPATH];
	char           *path;
	const char	*gamename;

    gamename = (net_compatibility->value) ? "game.so" : "gamexp.so";

	setreuid(getuid(), getuid());
	setegid(getgid());

	if (game_library)
		Com_Error(ERR_FATAL, "Sys_GetGameAPI without Sys_UnloadingGame");

	Com_Printf("Loading %s\n\n", gamename);

	/* now run through the search paths */
	path = NULL;
	while (1) {
		path = FS_NextPath(path);
		if (!path)
			return NULL;	/* couldn't find one anywhere */
			
      		Com_Printf("Game dir: %s\n\n", path);
		
		Com_sprintf(name, sizeof(name), "%s/%s", path, gamename);

		/* skip it if it just doesn't exist */
		fp = fopen(name, "rb");
		if (fp == NULL)
			continue;
		fclose(fp);
		
		game_library = dlopen(name, RTLD_LAZY);
		
		if (game_library) {
			Com_DPrintf("LoadLibrary (%s)\n", name);
			break;
		} else {
#if defined __GNUC__ >= 3
			char  *str_p;
			
			Com_Printf("LoadLibrary (%s):", name);

			path = (char *)dlerror();
			str_p = strchr(path, ':');	/* skip the path (already shown) */
			if (str_p == NULL)
				str_p = path;
			else
				str_p++;

			Com_Printf("%s\n", str_p);

			return NULL;
#else
			Com_DPrintf("LoadLibrary (%s) failed\n", name, dlerror());
#endif
		}
	}

	GetGameAPI = (void *)dlsym(game_library, "GetGameAPI");

	if (!GetGameAPI) {
		Sys_UnloadGame();
		return NULL;
	}
	return GetGameAPI(parms);
}

/*****************************************************************************/

void
Sys_AppActivate(void)
{
}

void IN_Update(void);
void
Sys_SendKeyEvents(void)
{
    IN_Update();

	/* grab frame time  */
	sys_frame_time = Sys_Milliseconds();
}

// FIXME: implement if necessary
char *Sys_GetClipboardData(void) {
    return NULL;
}

/*****************************************************************************/

#ifdef __linux__

void
printBacktrace(int sig)
{
	void *array[15];
	size_t size;
	char **strings;
	int i;

	size = backtrace(array, 15);
	strings = backtrace_symbols(array, size);

	printf("Signal:       %i\n", sig);
	printf("\nBacktrace:\n");

	for (i = 0; i < size; i++)
	{
		printf("  %s\n", strings[i]);
	}

	printf("\n");
}

#else

void
printBacktrace(int sig)
{
	printf("Product:      Yamagi Quake II\n");
	printf("Version:      %4.2f\n", VERSION);
	printf("Plattform:    %s\n", BUILDSTRING);
	printf("Architecture: %s\n", CPUSTRING);
	printf("Signal:       %i\n", sig);
	printf("\nBacktrace:\n");
	printf("  Not available on this plattform.\n\n");

}

#endif

void
signalhandler(int sig)
{
	printf("\nYamagi Quake II crashed! This should not happen...\n");
	printf("\nMake sure that you're using the last version. It can\n");
	printf("be found at http://www.yamagi.org/quake2. If you do send\n");
	printf("a bug report to quake2@yamagi.org and include this output,\n");
	printf("detailed condition, reproduction of the issue (if possible)\n");
	printf("and additional data you think might be useful. Thanks.\n");
	printf("\n=======================================================\n\n");

#ifdef __linux__
	printBacktrace(sig);
#endif

    /* reset signalhandler */
	signal(SIGSEGV, SIG_DFL);
	signal(SIGILL, SIG_DFL);
	signal(SIGFPE, SIG_DFL);
    signal(SIGABRT, SIG_DFL); 

	/* pass signal to the os */
    raise(sig);
}

void
registerHandler(void)
{
	signal(SIGSEGV, signalhandler);
	signal(SIGILL, signalhandler);
	signal(SIGFPE, signalhandler);
    signal(SIGABRT, signalhandler);
}

int
main(int argc, char **argv)
{
	int	mytime, oldtime, newtime;

    // TODO: signal handlers for ignoring FPU exceptions and printing backtraces
    registerHandler();

	/* go back to real user for config loads */
	saved_euid = geteuid();
	seteuid( getuid() );

	Qcommon_Init(argc, argv);
	oldtime = Sys_Milliseconds();

	while (1) {
		/* find time spent rendering last frame */
		do {
			newtime = Sys_Milliseconds();
			mytime = newtime - oldtime;
		} while (mytime < 1);

// TODO: implement
#if 0
		// Heffo - AVI Export
		if(avi_fps && avi_fps->value)
		{
			curtime += (1000/avi_fps->value);
			Qcommon_Frame(1000/avi_fps->value);
		} else
#endif
			Qcommon_Frame(mytime);

		oldtime = newtime;
	}

    // never gets here
    return 0;
}
