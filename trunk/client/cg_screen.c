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
// cl_scrn.c -- master for refresh, status bar, console, chat, notify, etc

/*

  full screen console
  put up loading plaque
  blanked background with loading plaque
  blanked background with menu
  cinematics
  full screen image for quit and victory

  end of unit intermissions

  */

#include "client.h"
#include "../renderer/r_local.h"

float scr_con_current;			// aproaches scr_conlines at scr_conspeed
float scr_conlines;				// 0.0 to 1.0 lines of console to display

bool scr_initialized;		// ready to draw

int scr_draw_loading;

vrect_t scr_vrect;				// position of render window on screen


cvar_t *scr_viewsize;
cvar_t *scr_conspeed;
cvar_t *scr_centertime;
cvar_t *scr_showturtle;
cvar_t *scr_showpause;
cvar_t *scr_printspeed;

cvar_t *scr_netgraph;
cvar_t *scr_timegraph;
cvar_t *scr_graphheight;
cvar_t *scr_graphscale;
cvar_t *scr_graphshift;
cvar_t *scr_drawall;

extern cvar_t *cl_drawTime;	// JKnife -- HUD Clock

typedef struct {
	int x1, y1, x2, y2;
} dirty_t;

dirty_t scr_dirty, scr_old_dirty[2];

char crosshair_pic[MAX_QPATH];
int crosshair_width, crosshair_height;

void SCR_TimeRefresh_f (void);
void SCR_Loading_f (void);
/*
===============================================================================

CENTER PRINTING

===============================================================================
*/

char scr_centerstring[1024];
float scr_centertime_start;		// for slow victory printing
float scr_centertime_off;
int scr_center_lines;
int scr_erase_center;

void CL_AddAltString(int x, int y, int scale, char *s) {

	while (*s) {
		R_AddCharsToList(x, y, scale, *s ^ 0x80, gi.menuFont);
		x += 8 * scale * FONT_INTERVAL;
		s++;
	}
}

void CL_AddString(int x, int y, int scale, char *s, image_t *inTex) {

	while (*s) {
		R_AddCharsToList(x, y, scale, *s, inTex);
		x += 8 * scale * FONT_INTERVAL;
		s++;
	}
}

/*
==============
SCR_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void SCR_CenterPrint (char *str) {
	char *s;
	char line[64];
	int i, j, l;

	strncpy (scr_centerstring, str, sizeof(scr_centerstring)-1);
	scr_centertime_off = scr_centertime->value;
	scr_centertime_start = cl.time;

	// count the number of lines for centering
	scr_center_lines = 1;
	s = str;
	while (*s) {
		if (*s == '\n')
			scr_center_lines++;
		s++;
	}

	// echo it to the console
	Com_Printf
		("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");

	s = str;
	do {
		// scan the width of the line
		for (l = 0; l < 40; l++)
		if (s[l] == '\n' || !s[l])
			break;
		for (i = 0; i < (40 - l) * 0.5; i++)
			line[i] = ' ';

		for (j = 0; j < l; j++) {
			line[i++] = s[j];
		}

		line[i] = '\n';
		line[i + 1] = 0;

		Com_Printf ("%s", line);

		while (*s && *s != '\n')
			s++;

		if (!*s)
			break;
		s++;					// skip the \n
	} while (1);
	Com_Printf
		("\n\n\35\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\36\37\n\n");
	Con_ClearNotify ();
}


void SCR_DrawCenterString (void) {
	char *start;
	int l;
	int j;
	int x, y;
	int remaining;

	// the finale prints the characters one at a time
	remaining = 9999;

	scr_erase_center = 0;
	start = scr_centerstring;

	if (scr_center_lines <= 4)
		y = viddef.height * 0.35;
	else
		y = 48;

	do {
		// scan the width of the line
		for (l = 0; l < 40; l++)
			if (start[l] == '\n' || !start[l])
				break;

		x = (viddef.width - l * 6 * ui_fontScale->integer) * 0.5;

		SCR_AddDirtyPoint (x, y);

		for (j = 0; j < l; j++, x += 6 * ui_fontScale->integer) {
			
			R_AddCharsToList(x, y, ui_fontScale->integer, start[j], gi.consFont);

			if (!remaining--)
				return;
		}
		SCR_AddDirtyPoint (x, y + 6 * ui_fontScale->integer);

		y += 8 * ui_fontScale->integer;

		while (*start && *start != '\n')
			start++;

		if (!*start)
			break;

		start++;				// skip the \n
	} while (1);
}

void SCR_CheckDrawCenterString (void) {
	scr_centertime_off -= cls.frameTime;

	if (scr_centertime_off <= 0)
		return;

	SCR_DrawCenterString ();
}

//=============================================================================

/*
=================
SCR_CalcVrect

Sets scr_vrect, the coordinates of the rendered window
=================
*/
static void SCR_CalcVrect (void) {
	int size;

	// bound viewsize
	if (scr_viewsize->value < 40)
		Cvar_Set ("viewsize", "40");
	if (scr_viewsize->value > 100)
		Cvar_Set ("viewsize", "100");

	size = scr_viewsize->value;

	scr_vrect.width = viddef.width * size / 100;
	scr_vrect.width &= ~7;

	scr_vrect.height = viddef.height * size / 100;
	scr_vrect.height &= ~1;

	scr_vrect.x = (viddef.width - scr_vrect.width) * 0.5;
	scr_vrect.y = (viddef.height - scr_vrect.height) * 0.5;
}


/*
=================
SCR_SizeUp_f

Keybinding command
=================
*/
void SCR_SizeUp_f (void) {
	Cvar_SetValue ("viewsize", scr_viewsize->value + 10);
}


/*
=================
SCR_SizeDown_f

Keybinding command
=================
*/
void SCR_SizeDown_f (void) {
	Cvar_SetValue ("viewsize", scr_viewsize->value - 10);
}

//============================================================================

/*
==================
SCR_Init
==================
*/
void SCR_Init (void) {
	scr_viewsize = Cvar_Get ("viewsize", "100", CVAR_ARCHIVE);
	scr_conspeed = Cvar_Get ("scr_conspeed", "3", 0);
	scr_showturtle = Cvar_Get ("scr_showturtle", "0", 0);
	scr_showpause = Cvar_Get ("scr_showpause", "1", 0);
	scr_centertime = Cvar_Get ("scr_centertime", "2.5", 0);
	scr_printspeed = Cvar_Get ("scr_printspeed", "8", 0);
	scr_netgraph = Cvar_Get ("netgraph", "0", 0);
	scr_timegraph = Cvar_Get ("timegraph", "0", 0);
	scr_graphheight = Cvar_Get ("graphheight", "32", 0);
	scr_graphscale = Cvar_Get ("graphscale", "1", 0);
	scr_graphshift = Cvar_Get ("graphshift", "0", 0);
	scr_drawall = Cvar_Get ("scr_drawall", "0", 0);

	//
	// register our commands
	//
	Cmd_AddCommand ("timerefresh", SCR_TimeRefresh_f);
	Cmd_AddCommand ("loading", SCR_Loading_f);
	Cmd_AddCommand ("sizeup", SCR_SizeUp_f);
	Cmd_AddCommand ("sizedown", SCR_SizeDown_f);

	scr_initialized = true;
}


/*
==============
SCR_DrawNet
==============
*/
void SCR_DrawNet (void) {
	if (cls.netchan.outgoing_sequence - cls.netchan.incoming_acknowledged
		< CMD_BACKUP - 1)
		return;

	Draw_ScaledPic(scr_vrect.x, scr_vrect.y, ui_fontScale->value, ui_fontScale->value, 0, i_net, "null");
}

/*
==============
SCR_DrawPause
==============
*/
void SCR_DrawPause (void) {

	if (!scr_showpause->integer)	// turn off for screenshots
		return;

	if (!cl_paused->integer)
		return;
	
	if (cls.menuActive)
		return;

	Draw_ScaledPic(	(float)viddef.width		* 0.5 - (float)i_pause[0]->width	* 0.5 - 7.0 * ui_fontScale->value,
					(float)viddef.height	* 0.5 - (float)i_pause[1]->height	* 0.5 + 56.0,
					ui_fontScale->value, ui_fontScale->value, PF_LIGHT, i_pause[0], i_pause[1]);
}

/*
==============
SCR_DrawLoading
==============
*/

void SCR_DrawLoadingBar (float percent, float scale) {

	Draw_Fill (2, viddef.height - scale * 10 + 3, viddef.width * percent * 0.01, scale * 3 - 6, 1.0, 1.0, 1.0, 1.0, true);

}

#include "../renderer/r_local.h"

void SCR_DrawLoading (void) {
	int		scaled, center;
	char	mapfile[32], saveshot[32];
	char	*mapname;
	int		fontscale = (int)ui_fontScale->integer;

	if (!scr_draw_loading)
		return;

	scr_draw_loading = 0;
	
	R_Flush2D();

	if (loadingMessage && cl.configstrings[CS_MODELS + 1][0]) {
		

		strcpy (mapfile, cl.configstrings[CS_MODELS + 1] + 5);	// skip "maps/"
		mapfile[strlen (mapfile) - 4] = 0;	// cut off ".bsp"
		
		if (drawSaveShot[0]){

		strcpy(saveshot, va("/savexp/%s/shot.jpg", drawSaveShot));
		// start from level autosave
		if (!Q_strcasecmp(drawSaveShot, "save0")){
			if (Draw_FindPic(va("/levelshots/%s.jpg", mapfile)))
				Draw_StretchPic(0, 0, viddef.width, viddef.height, PF_LOADSCREEN | PF_SCANLINE | PF_VIGNETTE | PF_NOALPHA, va("/levelshots/%s.jpg", mapfile), "null");
			else
				Draw_StretchPic(0, 0, viddef.width, viddef.height, PF_LOADSCREEN | PF_SCANLINE | PF_VIGNETTE | PF_NOALPHA, "/gfx/defshot.jpg", "null");
		}else // draw save shot
			if (Draw_FindPic(va("/savexp/%s/shot.jpg", drawSaveShot)))
				Draw_StretchPic(0, 0, viddef.width, viddef.height, PF_LOADSCREEN | PF_SCANLINE | PF_VIGNETTE | PF_NOALPHA, saveshot, "null");
			else // cant find any shots
				Draw_StretchPic(0, 0, viddef.width, viddef.height, PF_LOADSCREEN | PF_SCANLINE | PF_VIGNETTE | PF_NOALPHA, "/gfx/defshot.jpg", "null");
			}
			else {
			if (Draw_FindPic(va("/levelshots/%s.jpg", mapfile)))
				Draw_StretchPic(0, 0, viddef.width, viddef.height, PF_LOADSCREEN | PF_SCANLINE | PF_VIGNETTE | PF_NOALPHA, va("/levelshots/%s.jpg", mapfile), "null");
			else
				Draw_StretchPic(0, 0, viddef.width, viddef.height, PF_LOADSCREEN | PF_SCANLINE | PF_VIGNETTE | PF_NOALPHA, "/gfx/defshot.jpg", "null");
		}

		scaled = 8 * fontscale;
		SCR_DrawLoadingBar (loadingPercent, scaled);

		mapname = cl.configstrings[CS_NAME];
		
		center = viddef.width / 2 - (int)strlen(mapname) * fontscale * 6;
		RE_SetColor(colorGreen);

		CL_AddString(center, 20 * fontscale, fontscale * 2, mapname, gi.consFont);
		
		RE_SetColor (colorYellow);
		CL_AddString(0, 44 * fontscale, fontscale, va ("%s", loadingMessages[0]), gi.consFont);
		CL_AddString(0, 54 * fontscale, fontscale, va ("%s", loadingMessages[1]), gi.consFont);
		CL_AddString(0, 64 * fontscale, fontscale, va ("%s", loadingMessages[2]), gi.consFont);
		CL_AddString(0, 74 * fontscale, fontscale, va ("%s", loadingMessages[3]), gi.consFont);
		RE_SetColor (colorWhite);		
	}
}



//=============================================================================

/*
==================
SCR_RunConsole

Scroll it up or down
==================
*/
void SCR_RunConsole (void) {
	// decide on the height of the console
	if (cls.key_dest == key_console) {
		
		if (!com_fullConsole->integer)
			scr_conlines = 0.25;
		else
			scr_conlines = 0.5;		// half screen
	}
	else
		scr_conlines = 0;		// none visible

	if (scr_conlines < scr_con_current) {
		scr_con_current -= scr_conspeed->value * cls.frameTime;
		if (scr_conlines > scr_con_current)
			scr_con_current = scr_conlines;

	}
	else if (scr_conlines > scr_con_current) {
		scr_con_current += scr_conspeed->value * cls.frameTime;
		if (scr_conlines < scr_con_current)
			scr_con_current = scr_conlines;
	}

}

/*
==================
SCR_DrawConsole
==================
*/
void SCR_DrawConsole (void) {
	Con_CheckResize ();
	
	if (cls.state == ca_disconnected || cls.state == ca_connecting) {	// forced  full screen console
		Con_DrawConsole (1.0);
		return;
	}

	if (cls.state != ca_active || !cl.refresh_prepped) {	// connected, but can't render
		float size = 0.5;
		Con_DrawConsole (size);
		Draw_Fill (0, viddef.height * size, viddef.width, viddef.height * size, 0.0, 0.0, 0.0, 1.0, false);
		return;
	}

	if (scr_con_current) {
		Con_DrawConsole (scr_con_current);
	}
	else {
		if (cls.key_dest == key_game || cls.key_dest == key_message)
			Con_DrawNotify ();	// only draw notify in game
	}
}

//=============================================================================

/*
================
SCR_BeginLoadingPlaque
================
*/

bool needLoadingPlaque (void) {
	if (!cls.disableScreen || !scr_draw_loading)
		return true;
	return false;
}


void SCR_BeginLoadingPlaque (void) {
	
	S_StopAllSounds ();
	
	cl.sound_prepped = false;	// don't play ambients
	
	Music_Stop ();
	
	if (cls.disableScreen)
		return;

	if (developer->integer)
		return;

	if (cls.state == ca_disconnected)
		return;					// if at console, don't bring up the
	// plaque
	if (cls.key_dest == key_console)
		return;
	if (cl.cinematictime > 0)
		scr_draw_loading = 2;	// clear to balack first
	else
		scr_draw_loading = 1;

	SCR_UpdateScreen ();
	
	cls.disableScreen = Sys_Milliseconds ();
	
	cls.disableServerCount = cl.servercount;
}

/*
================
SCR_EndLoadingPlaque
================
*/
void SCR_EndLoadingPlaque (void) {
	cls.disableScreen = 0;
	scr_draw_loading = 0;
	Con_ClearNotify ();
}

/*
================
SCR_Loading_f
================
*/
void SCR_Loading_f (void) {
	SCR_BeginLoadingPlaque ();
}

/*
================
SCR_TimeRefresh_f
================
*/
int entitycmpfnc (const entity_t * a, const entity_t * b) {
	/*
	 ** all other models are sorted by model then skin
	 */
	if (a->model == b->model) {
		return ((int)a->skin - (int)b->skin);
	}
	else {
		return ((int)a->model - (int)b->model);
	}
}

void SCR_TimeRefresh_f (void) {
	int i;
	int start, stop;
	float time;

	if (cls.state != ca_active)
		return;

	start = Sys_Milliseconds ();

	if (Cmd_Argc () == 2) {		// run without page flipping
		R_BeginFrame ();
		for (i = 0; i < 128; i++) {
			cl.refdef.viewangles[1] = i / 128.0 * 360.0;
			R_RenderFrame (&cl.refdef);
		}
		GLimp_EndFrame ();
	}
	else {
		for (i = 0; i < 128; i++) {
			cl.refdef.viewangles[1] = i / 128.0 * 360.0;

			R_BeginFrame ();
			R_RenderFrame (&cl.refdef);
			GLimp_EndFrame ();
		}
	}

	stop = Sys_Milliseconds ();
	time = (stop - start) / 1000.0;
	Com_Printf ("%f seconds ("S_COLOR_YELLOW"%f" S_COLOR_WHITE " fps)\n", time, 128 / time);
}

/*
=================
SCR_AddDirtyPoint
=================
*/
void SCR_AddDirtyPoint (int x, int y) {
	if (x < scr_dirty.x1)
		scr_dirty.x1 = x;
	if (x > scr_dirty.x2)
		scr_dirty.x2 = x;
	if (y < scr_dirty.y1)
		scr_dirty.y1 = y;
	if (y > scr_dirty.y2)
		scr_dirty.y2 = y;
}

void SCR_DirtyScreen (void) {
	SCR_AddDirtyPoint (0, 0);
	SCR_AddDirtyPoint (viddef.width - 1, viddef.height - 1);
}

//===============================================================


#define STAT_MINUS		10		// num frame for '-' stats digit
char *sb_nums[2][11] = { 
	{ "num_0",	"num_1",  "num_2",  "num_3",  "num_4",  "num_5",  "num_6",  "num_7",  "num_8",  "num_9",  "num_minus" },
	{ "anum_0", "anum_1", "anum_2", "anum_3", "anum_4",	"anum_5", "anum_6", "anum_7", "anum_8", "anum_9", "anum_minus" }
};

char *sb_nums_bump[11] = {	"num_0_bump", "num_1_bump", "num_2_bump", "num_3_bump", "num_4_bump", "num_5_bump",
							"num_6_bump", "num_7_bump", "num_8_bump", "num_9_bump", "num_minus_bump" };


#define	ICON_WIDTH	24
#define	ICON_HEIGHT	24
#define	CHAR_WIDTH	16
#define	ICON_SPACE	8


/*
================
SizeHUDString

Allow embedded \n in the string
================
*/
void SizeHUDString (char *string, int *w, int *h) {
	int lines, width, current;

	lines = 1;
	width = 0;

	current = 0;
	while (*string) {
		if (*string == '\n') {
			lines++;
			current = 0;
		}
		else {
			current++;
			if (current > width)
				width = current;
		}
		string++;
	}

	*w = width * 8;
	*h = lines * 8;
}

void CL_AddHUDString(float x, float y, int scale, int centerwidth, int xor, char *string) {
	float	margin;
	char	line[1024];
	int		width;
	int		i;

	margin = x;

	while (*string) {
		// scan out one line of text from the string
		width = 0;
		while (*string && *string != '\n')
			line[width++] = *string++;
		line[width] = 0;

		if (centerwidth)
			x = centerwidth - (width * 8 * scale * FONT_INTERVAL) / 2;
		else
			x = margin;

		for (i = 0; i < width; i++) {
			if(xor)
				R_AddCharsToList(x, y, scale, line[i] ^ xor, gi.menuFont);
			else
				R_AddCharsToList(x, y, scale, line[i] ^ xor, gi.consFont);
			x += 8 * scale * FONT_INTERVAL;
		}
		if (*string) {
			string++;	// skip the \n
			x = margin;
			y += 8 * scale;
		}
	}
}


/*
==============
SCR_DrawField
==============
*/
void SCR_DrawField (int x, int y, float scale_x, float scale_y, int color, int width, int value) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if (width < 1)
		return;
	
	// draw number string
	if (width > 5)
		width = 5;

	SCR_AddDirtyPoint (x, y);
	SCR_AddDirtyPoint (x + (width*CHAR_WIDTH + 2)*scale_x, y + 23 * scale_y);

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen (num);
	if (l > width)
		l = width;
	x += (2 + CHAR_WIDTH*(width - l))*scale_x;

	ptr = num;

	while (*ptr && l) {
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr - '0';

		Draw_PicScaled (x, y, scale_x, scale_y, PF_LIGHT, sb_nums[color][frame], sb_nums_bump[frame]);
		x += CHAR_WIDTH*scale_x;
		ptr++;
		l--;
	}
}



/*
===============
SCR_TouchPics

Allows rendering code to cache all needed sbar graphics
===============
*/

void SCR_TouchPics (void) {
	int i, j;

	for (i = 0; i < 2; i++)
	for (j = 0; j < 11; j++)
		Draw_FindPic (sb_nums[i][j]);
	
	for (j = 0; j < 11; j++)
		Draw_FindPic(sb_nums_bump[j]);

	if (crossHair->integer) {
		if (crossHair->integer > 8 || crossHair->integer < 0)
			crossHair->integer = 8;

		Com_sprintf (crosshair_pic, sizeof(crosshair_pic), "chx%i", (int)(crossHair->value));
		Draw_GetPicSize (&crosshair_width, &crosshair_height, crosshair_pic);

		if (!crosshair_width)
			crosshair_pic[0] = 0;

	}
}
//=======================================================

/*
==================
SCR_UpdateScreen

This is called every frame, and can also be called explicitly to flush
text to the screen.
==================
*/
extern cvar_t *r_lightEditor;

extern cvar_t *r_speeds;
void SCR_DrawStats ();
void SCR_DrawLayout (void);

extern cvar_t *cl_drawFPS;

void SCR_DrawSpeeds(void) {

	char	bspTris[64],			litBspTris[64],			aliasTris[64],		litAliasTris[64],  
			dynamicShadowTris[64],	partTris[64],			numDynShadow[64],	decalTris[64],
			numVisLights[64],		staticShadowTris[64],	numDips[64];

	int heightScale = viddef.height * 0.25;
	int	scale = ui_fontScale->integer * 8 * FONT_INTERVAL;
	int fontScale = ui_fontScale->integer;

	if (!r_speeds->integer || !cl.refresh_prepped)
		return;

	sprintf(bspTris, "%i Bsp Tris", c_brushTris);
	sprintf(litBspTris, "%i Lit Bsp Tris", c_lightBrushTris);
	sprintf(aliasTris, "%i Alias Tris", c_aliasTris);
	sprintf(litAliasTris, "%i Lit Alias Tris", c_litAliasTris);
	sprintf(dynamicShadowTris, "%i Dynamic Shadow Tris", c_numDynamicShadowsTris);
	sprintf(staticShadowTris, "%i Static Shadow Tris", c_staticShadowTris);
	sprintf(numDynShadow, "%i Dynamic Shadows", c_numDynamicShadows);
	sprintf(numVisLights, "%i Vis Lights", c_numVisLights);
	sprintf(partTris, "%i Particles Tris", c_particlesTris);
	sprintf(decalTris, "%i Decals Tris", c_decalsTris);
	sprintf(numDips, "%i Draw Calls", c_numDips);

	RE_SetColor(colorGreen);

	CL_AddString(viddef.width - ((int)strlen(bspTris) + 1)		* scale, heightScale, fontScale, bspTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(litBspTris) + 1)	* scale, heightScale + 10 * fontScale, fontScale, litBspTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(aliasTris) + 1)	* scale, heightScale + 20 * fontScale, fontScale, aliasTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(litAliasTris) + 1) * scale, heightScale + 30 * fontScale, fontScale, litAliasTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(dynamicShadowTris) + 1)	* scale, heightScale + 40 * fontScale, fontScale, dynamicShadowTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(staticShadowTris) + 1)		* scale, heightScale + 50 * fontScale, fontScale, staticShadowTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(numDynShadow) + 1) * scale, heightScale + 60 * fontScale, fontScale, numDynShadow, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(numVisLights) + 1) * scale, heightScale + 70 * fontScale, fontScale, numVisLights, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(partTris) + 1)		* scale, heightScale + 80 * fontScale, fontScale, partTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(decalTris) + 1)	* scale, heightScale + 90 * fontScale, fontScale, decalTris, gi.consFont);
	CL_AddString(viddef.width - ((int)strlen(numDips) + 1)		* scale, heightScale + 100 * fontScale, fontScale, numDips, gi.consFont);

	RE_SetColor (colorWhite);
}

#ifdef _WIN32
void GLimp_GetCpuUtilization(unsigned* cpuUtil);
extern 	uint sys_numCpuCores;
#endif

void SCR_DrawCpuUtilization() {
	static	char	cpuUtil[21] = { 0 };
	static	int		frame = 0, delta = 4, lastUpdate;
	static	uint	procUtil;
	int		fontScale = ui_fontScale->integer;
	int		scale = 8 * fontScale * FONT_INTERVAL;

	if (!sys_cpuUtilization->integer || !cl.refresh_prepped)
		return;

#ifdef _WIN32
	frame++;

	if (curtime - lastUpdate >= 1000 / delta) {
		lastUpdate = curtime;
		frame = 0;
		GLimp_GetCpuUtilization(&procUtil);
	}

	if (cls.state == ca_active) {
		Com_sprintf(cpuUtil, sizeof(cpuUtil), "%3d%c CPU Utilization", (int)procUtil / sys_numCpuCores, 37); // ASCII code of % is 37
		int cpuUtilLengh = (int)strlen(cpuUtil);
		cpuUtilLengh += 1;

		CL_AddString(viddef.width - cpuUtilLengh * scale, viddef.height * 0.65 - 60, fontScale, cpuUtil, gi.consFont);

	}
#endif
}

void SCR_DrawFPS (void) {
	static	char	avrfps[10] = { 0 }, minfps[22] = { 0 }, frameTime[20] = { 0 };
	static	int		fps = 0;
	static	int		lastUpdate;
	const	int		delta = 4;
	static	int		fpsAvg = 0;
	int	fontScale = ui_fontScale->integer;
	int scale = 8 * ui_fontScale->integer * FONT_INTERVAL;
	
	if (cls.state != ca_active || !cl.refresh_prepped || !ui_drawFPS->integer)
		return;

	fps++;

	if (curtime - lastUpdate >= 1000 / delta) {

		const float alpha = 0.45;

		if (cl.minFps == 0) // only one time per level
			cl.minFps = 999999;

		if (fpsAvg < cl.minFps)
			cl.minFps = fpsAvg;

		if (fpsAvg > cl.maxFps)
			cl.maxFps = fpsAvg;

			Com_sprintf(minfps, sizeof(minfps), "Min/Max %3d/%3d Fps", cl.minFps, cl.maxFps);
			Com_sprintf(avrfps, sizeof(avrfps), "%4d Fps", (int)fpsAvg);

		lastUpdate = curtime;
		fpsAvg = delta * (alpha * fps + ((1 - alpha) * fpsAvg) / delta);
		fps = 0;
	}

	int avrFpsLengh = (int)strlen(avrfps);
	int minFpsLengh = (int)strlen(minfps);
	avrFpsLengh += 1;
	minFpsLengh += 1;

	if (ui_drawFPS->integer == 2) {
		CL_AddString(viddef.width - avrFpsLengh * scale, viddef.height * 0.65 - 40, fontScale, avrfps, gi.consFont);
		CL_AddString(viddef.width - minFpsLengh * scale, viddef.height * 0.65 - 20, fontScale, minfps, gi.consFont);
	} else
		CL_AddString(viddef.width - avrFpsLengh * scale, viddef.height * 0.65, fontScale, avrfps, gi.consFont);
}

void SRC_DrawFrameTime(int start, int stop) {

	if (ui_drawFPS->integer == 2 && (cls.state == ca_active) && cl.refresh_prepped) {
		static char	frameTime[22] = { 0 };
		static int frame = 0, lastUpdate, delta = 4;
		static float msec;
		int scale = 8 * ui_fontScale->integer * FONT_INTERVAL;

		frame++;
		if (curtime - lastUpdate >= 1000 / delta) {
			lastUpdate = curtime;
			frame = 0;
			msec = (float)stop - (float)start;
		}
		Com_sprintf(frameTime, sizeof(frameTime), "Frame Time %.1f Msec", msec);
		int frameTimeLenght = (int)strlen(frameTime);
		frameTimeLenght += 1;
		CL_AddString(viddef.width - frameTimeLenght * scale, viddef.height * 0.65, ui_fontScale->integer, frameTime, gi.consFont);
	}
}

const char *CL_NameForCompileFlags(int compileFlags)
{
	if (FBitSet(compileFlags, SURF_LIGHT))
		return "SURF_LIGHT";
	else if (FBitSet(compileFlags, SURF_SLICK))
		return "SURF_SLICK";
	else if (FBitSet(compileFlags, SURF_SKY))
		return "SURF_SKY";
	else if (FBitSet(compileFlags, SURF_WARP))
		return "SURF_WARP";
	else if (FBitSet(compileFlags, SURF_NODRAW))
		return "SURF_NODRAW";
	else if (FBitSet(compileFlags, SURF_TRANS33))
		return "SURF_TRANS33";
	else if (FBitSet(compileFlags, SURF_TRANS66))
		return "SURF_TRANS66";
	else if (FBitSet(compileFlags, SURF_FLOWING))
		return "SURF_FLOWING";
	else if (FBitSet(compileFlags, SURF_NODRAW))
		return "SURF_NODRAW";
	else if (FBitSet(compileFlags, SURF_ALPHA))
		return "SURF_ALPHA";
	else
		return "";
}
const char *CL_GetCompileFlags(int compileFlags)
{
	char	string[MAX_STRING_TOKENS];

	string[0] = 0;

	for (int i = 0; i < 31; i++) {

		int  checkbit = FBitSet(compileFlags, BIT(i));

		if (checkbit == 0)
			continue;

		strncat(string, CL_NameForCompileFlags(checkbit), MAX_STRING_TOKENS - strlen(string));
		strncat(string, "|", MAX_STRING_TOKENS - strlen(string));
	}

	int  total = strlen(string);

	// how this possible?
	if (total <= 0) return "";

	// remove last '|'
	string[total - 1] = '\0';

	return va("%s", string);
}
void SCR_ShowTexNames() {

	trace_t		trace;
	vec3_t		end, forward, right, up;

	if (!cl_showMaterials->integer)
		return;

	if (cls.state != ca_active || !cl.refresh_prepped)
		return;

	AngleVectors(cl.refdef.viewangles, forward, right, up);
	VectorMA(cl.refdef.vieworg, 4096, forward, end);
	trace = CL_PMTraceWorld(cl.refdef.vieworg, vec3_origin, vec3_origin, end, (MASK_SOLID | MASK_WATER), false);

	RE_SetColor(colorGreen);

	if (trace.surface->name[0]) {

		char	texName[MAX_TOKEN_CHARS], surfFlags[MAX_TOKEN_CHARS];

		Com_sprintf(texName, sizeof(texName), "Texture Name: %s", trace.surface->name);
		Com_sprintf(surfFlags, sizeof(surfFlags), "Flags: %s", CL_GetCompileFlags(trace.surface->flags));
		CL_AddString(0, viddef.height / 2 - 50, ui_fontScale->integer,texName, gi.consFont);
		CL_AddString(0, viddef.height / 2 - 25, ui_fontScale->integer, surfFlags, gi.consFont);
	}

	RE_SetColor(colorWhite);
}


extern cvar_t *r_mode;

void GL_MakeSaveShot(char* dir);
char makeSaveShot[16];
void SCR_DrawBatteryLevel();

void SCR_UpdateScreen (void) {
	// if the screen is disabled (loading plaque is up, or vid mode
	// changing)
	// do nothing at all
	int start = Sys_Milliseconds();

	if (cls.disableScreen) {
		if (cls.download)		// Knightmare- don't time out on downloads
			cls.disableScreen = Sys_Milliseconds ();
		if (Sys_Milliseconds () - cls.disableScreen > 120000 && cl.refresh_prepped && !(cl.cinematictime > 0)) {
			cls.disableScreen = 0;
			Com_Printf ("Loading plaque timed out.\n");
			return;
		}
		scr_draw_loading = 2;
	}

	if (!scr_initialized || !con.initialized)
		return;					// not initialized yet

	ui_fontScale->value = ClampCvar(2.0, 4.0, ui_fontScale->value);

	if(viddef.height <= 1024)
		Cvar_Set("ui_fontScale", "2");
	else
		Cvar_Set("ui_fontScale", "3");

	R_BeginFrame ();

	if (scr_draw_loading == 2) {	// loading plaque over black
		// screen
		R_SetPalette (NULL);
		SCR_DrawLoading ();

		if (cls.disableScreen)
			scr_draw_loading = 2;

		// NO FULLSCREEN CONSOLE!!!
		goto next;
	}
	// if a cinematic is supposed to be running, handle menus
	// and console specially
	else if (cl.cinematictime > 0) {
		if (cls.key_dest == key_menu) {
			if (cl.cinematicpalette_active) {
				R_SetPalette (NULL);
				cl.cinematicpalette_active = false;
			}
			M_Draw ();
		}
		else
			SCR_DrawCinematic ();
	}
	else {

		// make sure the game palette is active
		if (cl.cinematicpalette_active) {
			R_SetPalette (NULL);
			cl.cinematicpalette_active = false;
		}
	next:

		// do 3D refresh drawing, and then update the screen
		SCR_CalcVrect ();

		V_RenderView ();
		
		if (cls.state == ca_active && !scr_con_current)
			if (makeSaveShot[0])
			{
				GL_MakeSaveShot(makeSaveShot);
				makeSaveShot[0] = 0;
			}

		SCR_DrawStats ();

		if (!scr_con_current) {
			if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 1)
				SCR_DrawLayout ();
			if (cl.frame.playerstate.stats[STAT_LAYOUTS] & 2)
				CL_DrawInventory();
		}

		SCR_DrawNet ();
		SCR_CheckDrawCenterString ();
		SCR_DrawPause ();

#ifdef _WIN32
		if (cls.state == ca_active)
			SCR_DrawBatteryLevel();
#endif

		SCR_DrawFPS ();
		SCR_DrawCpuUtilization();
		SCR_ShowTexNames();
		
		int stop = Sys_Milliseconds();
		SRC_DrawFrameTime(start, stop);

		SCR_DrawConsole ();
		M_Draw ();
		SCR_DrawLoading ();
		SCR_DrawSpeeds();
	
		if (cls.state == ca_active && cl_playerPosition->integer && cl.refresh_prepped) {
			char pos[128];
			Com_sprintf(pos, sizeof(pos), "%i %i %i", (int)cl.refdef.vieworg[0], (int)cl.refdef.vieworg[1], (int)cl.refdef.vieworg[2]);
			CL_AddString(0, 8 * ui_fontScale->integer, ui_fontScale->integer, pos, gi.consFont);
		}
	}
	R_Flush2D();
	GLimp_EndFrame();
}
