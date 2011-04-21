/*
Copyright (C) 2002 Eric Lasota/Orbiter Productions

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

#include "client.h"
#include "windows.h"
#include "uix.h"

int uix = 0;
int uiy = 0;

cvar_t *ui_x;
cvar_t *ui_y;

uix_export_t uxe;
uix_import_t uxi;

uix_export_t *(*GetInterfaceAPI) (uix_import_t *import);

// This is platform-specific, used to load the UIX interface library
void UIX_APIExchange()
{
	HINSTANCE ui_library;

	ui_library = LoadLibrary("ui.dll");

	if(!ui_library)
	{
		GetInterfaceAPI = NULL;
	}
	else
	{
		GetInterfaceAPI = (void *)GetProcAddress (ui_library, "GetInterfaceAPI");
	}

	if(!GetInterfaceAPI)
		Com_Printf("*** Could not load UIX plug-in ***\n");
}

void UI_DrawString(const int x, const int y, const char *str, ...)
{
	int tx, ty;
	va_list		argz;
	char		result[4096];

	va_start (argz, str);
	vsprintf (result, str, argz);
	va_end (argz);

	tx = x;  ty = y;

	Draw_String(tx, ty, result);
}

void UI_DrawAltString(const int x, const int y, const char *str, ...)
{
	int tx, ty;
	va_list		argz;
	char		result[4096];

	va_start (argz, str);
	vsprintf (result, str, argz);
	va_end (argz);

	tx = x;  ty = y;

	DrawAltString(tx, ty, result);
}

// This is here so UIX can call with vargs
void UI_AddText(char *str, ...)
{
	va_list		argz;
	char		result[4096];

	va_start (argz, str);
	vsprintf (result, str, argz);
	va_end (argz);

	Cbuf_AddText(result);
}

// The API is going to bitch about const qualifiers, but this is C, so
// the compiler won't care.

// All UI cvars are set to zero or "" by default and archived
float UI_CvarValue (const char *cvarname)
{
	cvar_t *c;

	c = Cvar_Get(cvarname, "0", CVAR_ARCHIVE);
	return c->value;
}

const char UI_CvarString (const char *cvarname)
{
	cvar_t *c;

	c = Cvar_Get(cvarname, "0", CVAR_ARCHIVE);
	return c->string;
}

void UI_SetCvarValue (const char *cvarname, const float value)
{
	float tempf=value;

	Cvar_SetValue(cvarname, tempf);
}

void UI_SetCvarString (const char *cvarname, const char *value)
{
	char tempv[256];

	strcpy(tempv, value);

	Cvar_Set(cvarname, tempv);
}

void UI_ExitToConsole()
{
	cls.key_dest = key_console;
}

void UI_ExitDesktop()
{
	// Unpause the game if it's paused
	Cvar_Set("paused", "0");
	cls.key_dest = key_game;
}

// Just move the mouse
void UI_MoveMouse(const int x, const int y)
{
	uix = x;
	uiy = y;
}

void UI_PlaySound(const char *str)
{
//	S_StartLocalSound((char *)str);
}

const char *UI_GetBind(const int keynum)
{
	return keybindings[keynum];
}

qboolean loadedui = false;

char *Key_KeynumToString (int keynum);
int Key_StringToKeynum (char *str);
extern char	*keybindings[256];


// Because the refresh functions move...
void UI_DrawPic(int i, int j, char *c)
{
	Draw_Pic(i, j, c);
}

void UI_DrawStretchPic(int i, int j, int k, int l, char *c)
{
	Draw_StretchPic(i, j, k, l, c);
}


void UI_Init()
{
	uix_export_t *interfaced;

	ui_x = Cvar_Get("ui_x", "0.2", CVAR_ARCHIVE);
	ui_y = Cvar_Get("ui_y", "0.2", CVAR_ARCHIVE);

	// Exchange the library API
	UIX_APIExchange();

	// Set our functions
	uxi.DrawString = UI_DrawString;
	uxi.DrawAltString = UI_DrawAltString;
	uxi.DrawPic = UI_DrawPic;
	uxi.DrawStretchPic = UI_DrawStretchPic;
	uxi.Exec = UI_AddText;

	uxi.CvarValue = UI_CvarValue;
	uxi.CvarString = UI_CvarString;

	uxi.SetCvarValue = UI_SetCvarValue;
	uxi.SetCvarString = UI_SetCvarString;

	uxi.ExitDesktop = UI_ExitDesktop;
	uxi.ExitToConsole = UI_ExitToConsole;

	uxi.MoveMouse = UI_MoveMouse;

	uxi.PlaySound = UI_PlaySound;

	uxi.GetBind = UI_GetBind;
	uxi.SetBind = Key_SetBinding;

	uxi.StringToKeynum = Key_StringToKeynum;
	uxi.KeynumToString = Key_KeynumToString;

	if(GetInterfaceAPI)
		loadedui = true;
	else
		return;

	// Upload them to the library
	interfaced = GetInterfaceAPI(&uxi);

	if(!interfaced)
		Sys_Error("Failed to interface correctly with UI library");

	uxe = *interfaced;

	// Fake a draw, which won't do anything because it's not initialized yet,
	// but will update the screen size variables
	uxe.DrawObjects(uix, uiy, viddef.width, viddef.height);
	// Now initialize it
	uxe.Init();
}

void UI_Draw()
{
	if(cls.key_dest != key_menu)
		return;
	
	// Clamp the X/Y coordinates, since this is the only time the UI gets updated.
	if(uix < 0) uix = 0;
	if(uiy < 0) uiy = 0;
	if(uix >= viddef.width) uix = viddef.width - 1;
	if(uiy >= viddef.height) uiy = viddef.height - 1;

	if(loadedui)
		uxe.DrawObjects(uix, uiy, viddef.width, viddef.height);
	else
		cls.key_dest = key_console;
}

void UI_Keydown(int key)
{
	if(loadedui)
		uxe.KeyEvent(key);
}

void UI_Desktop()
{
	if(loadedui)
		cls.key_dest = key_menu;
	else
		Com_Printf("*** UIX not found, GUI not available ***\n");
}

void UI_MouseStatus(qboolean down)
{
	if(loadedui)
	{
		if(down)
			uxe.MouseDown();
		else
			uxe.MouseUp();
	}
}