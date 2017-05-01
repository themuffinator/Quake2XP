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
// winquake.h: Win32-specific Quake header file

#include <windows.h>

extern	HINSTANCE	global_hInstance;

extern HWND			cl_hwnd;
extern qboolean		ActiveApp, Minimized;

void IN_Activate(qboolean active);
void IN_MouseEvent(int mstate);

extern int		window_center_x, window_center_y;
extern RECT		window_rect;

cvar_t *xi_axisLx;
cvar_t *xi_axisLy;

cvar_t *xi_axisRx;
cvar_t *xi_axisRy;

cvar_t *xi_axisLt;
cvar_t *xi_axisRt;

cvar_t *xi_dpadArrowMap;
cvar_t *xi_useController;

extern qboolean xiActive;

void	IN_StartupXInput(void);
void	IN_ToggleXInput(cvar_t *var);
void	IN_ControllerMove(usercmd_t *cmd);
