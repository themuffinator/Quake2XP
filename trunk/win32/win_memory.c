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

#include "../qcommon/qcommon.h"
#include "winquake.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>

//===============================================================================

byte	*membase;
int		hunkmaxsize = 0;
int		cursize;
char	hunk_name[MAX_OSPATH];
int		hunkcount;
int		hunk_total_size = 0;

void* Hunk_Begin(int maxsize, char* name)
{
	// reserve a huge chunk of memory, but don't commit any yet
	cursize = 0;
	hunkmaxsize = maxsize;
	int l = strlen(name);
	if (l >= MAX_OSPATH)
		l = MAX_OSPATH - 1;
	memcpy(hunk_name, name, l);
	hunk_name[l] = 0;
	membase = (byte*)calloc(maxsize, 1);
	if (!membase)
		Sys_Error("Hunk_Begin: failed on reserving of %i bytes for %s", maxsize, hunk_name);
	return (void*)membase;
}

void* Hunk_Alloc(int size)
{
	// round to cacheline
	size = (size + 31) & ~31;
	cursize += size;
	if (cursize > hunkmaxsize)
		Sys_Error("Hunk_Alloc overflow on %s:\ncursize = %i hunkmaxsize = %i\nIf crashed during model or sprite loading,\ntry to clear cache and/or increase value of 'hunk_*' cvar...\nGamedir: %s", hunk_name, cursize, hunkmaxsize, FS_Gamedir());

	return (void*)(membase + cursize - size);
}


void Hunk_Free(void* base, int size)
{
	if (base)
		free(base);
	hunkmaxsize = 0;
	if (size)
	{
		hunk_total_size -= size;
		hunkcount--;
	}
}

// free the remaining unused virtual memory
byte* needFree;
int Hunk_End(char *name)
{
	byte* newbase;
	hunk_total_size += cursize;	// учтём размер для статистики
	hunkcount++;
	needFree = NULL;
	if (hunkmaxsize > cursize)
	{
		Com_DPrintf("Hunk_End: realloc from %imb to %ikb for %s\n", hunkmaxsize>>20, cursize>>10, name);
		newbase = (byte*)realloc(membase, cursize);
		if (newbase != membase)
		{	// случилась редкая хуйня: realloc при уменьшении блока памяти всё же переместил данные, сука!
			Com_DPrintf("Hunk_End: realloc() moved memory block, %s will be reload!\n", name);
			needFree = newbase;
			hunk_total_size -= cursize;	// не будем учитывать, фейл
			hunkcount--;
		}
	}
	hunkmaxsize = 0;
	return cursize;
}

//===============================================================================


/*
================
Sys_Milliseconds
================
*/

int	curtime;
int Sys_Milliseconds (void) {
	static int		base;
	static qboolean	initialized = qfalse;

	if (!initialized) {	// let base retain 16 bits of effectively random data
		base = timeGetTime () & 0xffff0000;
		initialized = qtrue;
	}

	curtime = timeGetTime () - base;

	return curtime;
}

void Sys_Mkdir (char *path) {
	_mkdir (path);
}
void Sys_ChDir (char *path) {
	_chdir (path);
}
//============================================

char	findbase[MAX_OSPATH];
char	findpath[MAX_OSPATH];
int		findhandle;

static qboolean CompareAttributes (unsigned found, unsigned musthave, unsigned canthave) {
	if ((found & _A_RDONLY) && (canthave & SFF_RDONLY))
		return qfalse;
	if ((found & _A_HIDDEN) && (canthave & SFF_HIDDEN))
		return qfalse;
	if ((found & _A_SYSTEM) && (canthave & SFF_SYSTEM))
		return qfalse;
	if ((found & _A_SUBDIR) && (canthave & SFF_SUBDIR))
		return qfalse;
	if ((found & _A_ARCH) && (canthave & SFF_ARCH))
		return qfalse;

	if ((musthave & SFF_RDONLY) && !(found & _A_RDONLY))
		return qfalse;
	if ((musthave & SFF_HIDDEN) && !(found & _A_HIDDEN))
		return qfalse;
	if ((musthave & SFF_SYSTEM) && !(found & _A_SYSTEM))
		return qfalse;
	if ((musthave & SFF_SUBDIR) && !(found & _A_SUBDIR))
		return qfalse;
	if ((musthave & SFF_ARCH) && !(found & _A_ARCH))
		return qfalse;

	return qtrue;
}

char *Sys_FindFirst (char *path, unsigned musthave, unsigned canthave) {
	struct _finddata_t findinfo;

	if (findhandle)
		Sys_Error ("Sys_BeginFind without close");
	findhandle = 0;

	COM_FilePath (path, findbase);
	findhandle = _findfirst (path, &findinfo);

	while ((findhandle != -1)) {
		if (CompareAttributes (findinfo.attrib, musthave, canthave)) {
			Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
			return findpath;
		}
		else if (_findnext (findhandle, &findinfo) == -1) {
			_findclose (findhandle);
			findhandle = -1;
		}
	}

	return NULL;
}

char *Sys_FindNext (unsigned musthave, unsigned canthave) {
	struct _finddata_t findinfo;

	if (findhandle == -1)
		return NULL;


	while (_findnext (findhandle, &findinfo) != -1) {
		if (CompareAttributes (findinfo.attrib, musthave, canthave)) {
			Com_sprintf (findpath, sizeof(findpath), "%s/%s", findbase, findinfo.name);
			return findpath;
		}
	}

	return NULL;
}

void Sys_FindClose (void) {
	if (findhandle != -1)
		_findclose (findhandle);
	findhandle = 0;
}


//============================================

