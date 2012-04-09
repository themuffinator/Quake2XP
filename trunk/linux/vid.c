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
/* Main windowed and fullscreen graphics interface module. This module */
/* is used for both the software and OpenGL rendering versions of the */
/* Quake refresh engine. */

#include "../client/client.h"
#include "../client/snd_loc.h"

/* Console variables that we need to access from this module */
cvar_t         *r_gamma;
cvar_t         *vid_ref;
cvar_t         *r_fullscreen;

/* Global variables used internally by this module */
viddef_t	viddef;		/* global video state; used by other modules */
qboolean	reflib_active = 0;

#define VID_NUM_MODES ( sizeof( vid_modes ) / sizeof( vid_modes[0] ) )

void IN_Close( void );

void IN_BackendInit( void );
void IN_BackendShutdown( void );

extern void VID_MenuShutdown ( void );

/*
 * ==========================================================================
 *
 * DLL GLUE
 *
 * ==========================================================================
 */

#define	MAXPRINTMSG	4096
void
VID_Printf(int print_level, char *fmt,...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if (print_level == PRINT_ALL)
		Com_Printf("%s", msg);
	else
		Com_DPrintf("%s", msg);
}

void
VID_Error(int err_level, char *fmt,...)
{
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start(argptr, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	Com_Error(err_level, "%s", msg);
}

/* ========================================================================== */

/*
 * ============ VID_Restart_f
 *
 * Console command to re-start the video mode and refresh DLL. We do this simply
 * by setting the modified flag for the vid_ref variable, which will cause
 * the entire video mode and refresh DLL to be reset on the next frame.
 * ============
 */
void
VID_Restart_f(void)
{
	vid_ref->modified = true;
}

/*
 * * VID_GetModeInfo
 */
typedef struct vidmode_s {
	const char     *description;
	int		width     , height;
	int		mode;
} vidmode_t;

static vidmode_t vid_modes[] =
{
     // generic screen
     { "800x600",   800, 600,   0 },      // 4:3
     { "1024x768",  1024, 768,  1 },      // 4:3
     { "1152x864",  1152, 864,  2 },      // 4:3
     { "1280x1024", 1280, 1024, 3 },      // 5:4
     { "1600x1200", 1600, 1200, 4 },      // 4:3
     { "2048x1536", 2048, 1536, 5 },      // 4:3
     // wide screen
     { "1280x720",  1280, 720,  6 },      // 16:9 720p HDTV
     { "1280x800",  1280, 800,  7 },      // 16:10
     { "1366x768",  1366, 768,  8 },      // 16:9, plasma & LCD TV
     { "1440x900",  1440, 900,  9 },      // 16:10
     { "1600x900",  1600, 900,  10 },     // 16:9 TV
     { "1680x1050", 1680, 1050, 11 },     // 16:10
     { "1920x1080", 1920, 1080, 12 },     // 16:9 1080p full HDTV
     { "1920x1200", 1920, 1200, 13 },     // 16:10
	 { "2560x1600", 2560, 1600, 14 },     // 16:10
	 { "Custom",    800,  600,  15}       // custom
};

qboolean
VID_GetModeInfo(int *width, int *height, int mode)
{
	if (mode < 0 || mode >= VID_NUM_MODES)
		return false;

	*width = vid_modes[mode].width;
	*height = vid_modes[mode].height;

	return true;
}

/*
 * * VID_NewWindow
 */
void
VID_NewWindow(int width, int height)
{
	viddef.width = width;
	viddef.height = height;

	cl.force_refdef = true;		// can't use a paused refdef
}

void
VID_FreeReflib(void)
{
    IN_Close();
    IN_BackendShutdown();
	reflib_active = false;
}

qboolean VID_StartRefresh()
{
	if (reflib_active) {
        IN_Close();
        IN_BackendShutdown();
        R_Shutdown();
		VID_FreeReflib();
	}

    IN_BackendInit();

	if (R_Init(0, 0) == -1)
	{
		R_Shutdown();
		VID_FreeReflib();
		return false;
	}

	Key_ClearStates();

	reflib_active = true;

	return true;
}


/*
 * ============ VID_CheckChanges
 *
 * This function gets called once just before drawing each frame, and it's sole
 * purpose in life is to check to see if any of the video mode parameters
 * have changed, and if they have to update the rendering DLL and/or video
 * mode to match. ============
 */
void
VID_CheckChanges(void)
{
	while (vid_ref->modified) {
		/*
		 * * refresh has changed
		 */

        cl.force_refdef = true;
		S_StopAllSounds();

		vid_ref->modified = false;
		r_fullscreen->modified = true;
		cl.refresh_prepped = false;
		cls.disable_screen = true;
        CL_ClearDecals();

		if (!VID_StartRefresh()) {
			Com_Error(ERR_FATAL, "Couldn't fall back to software refresh!");

			/*
			 * * drop the console if we fail to load a refresh
			 */
			if (cls.key_dest != key_console) 
			{
				Con_ToggleConsole_f();
			}
		}
		cls.disable_screen = false;
        CL_InitImages();
	}

}

/*
 * ============ VID_Init ============
 */
void
VID_Init(void)
{

	/*
	 * Create the video variables so we know how to start the graphics
	 * drivers
	 */
	/* if DISPLAY is defined, try X */
	r_fullscreen = Cvar_Get("r_fullscreen", "1", CVAR_ARCHIVE);
	r_gamma = Cvar_Get("r_gamma", "0.7", CVAR_ARCHIVE);

	/* Add some console commands that we want to handle */
	Cmd_AddCommand("vid_restart", VID_Restart_f);

	/* Start the graphics mode and load refresh DLL */
	VID_CheckChanges();
}

/*
 * ============ VID_Shutdown ============
 */
void
VID_Shutdown(void)
{
	if (reflib_active) {
        IN_Close();
        IN_BackendShutdown();
        R_Shutdown();
		VID_FreeReflib();
	}

    // FIXME: is it needed?
	VID_MenuShutdown();
}
