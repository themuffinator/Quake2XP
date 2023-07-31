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
// win_input.c -- windows xp raw mouse code

#include "../client/client.h"
#include "winquake.h"
#include "win_usbVendors.h"

extern	unsigned	sys_msg_time;

cvar_t	*m_inversion;
cvar_t	*v_centermove;
cvar_t	*v_centerspeed;
qboolean	in_appactive;


/*
============================================================

MOUSE CONTROL

============================================================
*/

int			mouse_oldbuttonstate;
POINT		current_pos;
int			mouse_x, mouse_y, old_mouse_x, old_mouse_y, mx_accum, my_accum;

int			old_x, old_y;

qboolean	mouseactive;	// qfalse when not focus app

qboolean	mouseinitialized;

int			window_center_x, window_center_y;
RECT		window_rect;


/*
===========
IN_ActivateMouse

Called when the window gains focus or changes in some way
===========
*/
void IN_ActivateMouse (void) {
	int		width, height;

	if (mouseactive)
		return;

	/// Berserker's fix
	width = GetSystemMetrics(SM_CXSCREEN);
	height = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(cl_hwnd, &window_rect);
	if (window_rect.left < 0)
		window_rect.left = 0;
	if (window_rect.top < 0)
		window_rect.top = 0;
	if (window_rect.right >= width)
		window_rect.right = width - 1;
	if (window_rect.bottom >= height - 1)
		window_rect.bottom = height - 1;

	window_center_x = (window_rect.right + window_rect.left) * 0.5;
	window_center_y = (window_rect.top + window_rect.bottom) * 0.5;

	old_x = window_center_x;
	old_y = window_center_y;

	SetCursorPos(window_center_x, window_center_y);
	SetCapture(cl_hwnd);
	ClipCursor(&window_rect);

	mouseactive = qtrue;
	while (ShowCursor(FALSE) >= 0);
}


/*
===========
IN_DeactivateMouse

Called when the window loses focus
===========
*/
void IN_DeactivateMouse (void) {
	
	if (!mouseactive)
		return;

	ClipCursor (NULL);
	ReleaseCapture ();

	mouseactive = qfalse;
	
	while (ShowCursor(TRUE) < 0);
}

#define MOUSE_BUTTONS 9

/*
===========
IN_StartupMouse
===========
*/
void IN_StartupMouse (void) {
	cvar_t		*cv;

	cv = Cvar_Get ("in_initmouse", "1", CVAR_NOSET);

	if (!cv->integer)
		return;

	mouseinitialized = qtrue;
}

/*
===========
IN_MouseEvent
===========
*/
void IN_MouseEvent (int mstate) {
	int		i;

	if (!mouseinitialized)
		return;

	// perform button actions
	for (i = 0; i < MOUSE_BUTTONS; i++) {
		if ((mstate & (1 << i)) &&
			!(mouse_oldbuttonstate & (1 << i))) {
			Key_Event (K_MOUSE1 + i, qtrue, sys_msg_time);
		}

		if (!(mstate & (1 << i)) &&
			(mouse_oldbuttonstate & (1 << i))) {
			Key_Event (K_MOUSE1 + i, qfalse, sys_msg_time);
		}
	}

	mouse_oldbuttonstate = mstate;
}


/*
===========
IN_MouseMove
===========
*/

void IN_PrintRawDevicesList() {

	PRAWINPUTDEVICELIST pRawInputDeviceList;
	UINT  numDevices;
	UINT  cbSize = sizeof(RAWINPUTDEVICELIST);

	Com_Printf("====== Init RAW Input Devices ======\n\n");

	if ((GetRawInputDeviceList(NULL, &numDevices, cbSize)) != 0){

		Com_Printf("\n" S_COLOR_RED "No RawInput devices attached\n");
		return;
	}
	else
		Com_DPrintf("" S_COLOR_YELLOW "... Found " S_COLOR_GREEN "%i" S_COLOR_YELLOW " RAW input devices.\n", numDevices);
	
	if ((pRawInputDeviceList = (PRAWINPUTDEVICELIST)malloc(cbSize * numDevices)) == NULL){

		Com_Printf("" S_COLOR_RED "Error mallocing RAWINPUTDEVICELIST\n");
		return;
	}

	if ((GetRawInputDeviceList(pRawInputDeviceList, &numDevices, cbSize)) == -1){

		Com_Printf("" S_COLOR_RED "1Error from GetRawInputDeviceList\n");
		free(pRawInputDeviceList);
		return;
	}

	GetRawInputDeviceList(pRawInputDeviceList, &numDevices, cbSize);

	for (int i = 0; i < numDevices; i++) {

		UINT             cbDataSize = 1000;
		RID_DEVICE_INFO  devInfo = { 0 };
		char             pData[1000] = { 0 };

		// For each device get the device name and then the device information
		cbDataSize = sizeof(pData);
		GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICENAME, pData, &cbDataSize);
		devInfo.cbSize = cbDataSize = sizeof(devInfo);  // specify the buffer size
		GetRawInputDeviceInfo(pRawInputDeviceList[i].hDevice, RIDI_DEVICEINFO, &devInfo, &cbDataSize);

		if (devInfo.dwType == RIM_TYPEHID)
			continue;

		char *pstart;
		char *vid_; // skip VID_
		char *pid_; // skip VID_XXXX&PID_
		char vid2[5] = { 0 };
		char pid2[5] = { 0 };

		if(devInfo.dwType == RIM_TYPEMOUSE)
			Com_Printf(S_COLOR_YELLOW"...Found Mouse:\n");
		if (devInfo.dwType == RIM_TYPEKEYBOARD)
			Com_Printf(S_COLOR_YELLOW"...Found Keyboard:\n");

		switch (devInfo.dwType) {

		case RIM_TYPEMOUSE:

			pstart = strstr(pData, "VID_");
			if (pstart) {
				vid_ = pstart + 4;
				pid_ = pstart + 13;

				strncpy(vid2, vid_, 4);
				strncpy(pid2, pid_, 4);

				DWORD value = strtoul(vid2, NULL, 16);
				int z;

				for (z = 0; z < NUM_VENDORS; z++) {
					if (value == usb_Vendors[z].vendorId) {
						Com_Printf("Vendor:           " S_COLOR_GREEN "%s\n", usb_Vendors[z].description);
						break;
					}
				}
					if (z == NUM_VENDORS)
						Com_Printf("Vendor:           " S_COLOR_GREEN "0x%04X\n", value);

				DWORD valPid = strtoul(pid2, NULL, 16);
				for (z = 0; z < NUM_INPUT_DEVICES; z++) {
					if (valPid == product[z].Id) {
						Com_Printf("Model:            " S_COLOR_GREEN "%s\n", product[z].description);
						break;
					}
				}
					if (z == NUM_INPUT_DEVICES)
						Com_Printf("Model:            " S_COLOR_GREEN "0x%04X\n", valPid);

						Com_Printf("Buttons:          " S_COLOR_GREEN "%d\n", devInfo.mouse.dwNumberOfButtons);
			}
			break;

		case RIM_TYPEKEYBOARD:
			
			pstart = strstr(pData, "VID_");
			if (pstart) {
				vid_ = pstart + 4;
				pid_ = pstart + 13;

				strncpy(vid2, vid_, 4);
				strncpy(pid2, pid_, 4);

				DWORD value = strtoul(vid2, NULL, 16);
				int z;

				for (z = 0; z < NUM_VENDORS; z++) {
					if (value == usb_Vendors[z].vendorId) {
						Com_Printf("Vendor:           " S_COLOR_GREEN "%s\n", usb_Vendors[z].description);
						break;
					}
				}
					if (z == NUM_VENDORS)
						Com_Printf("Vendor:           " S_COLOR_GREEN "0x%04X\n", value);

				DWORD valPid = strtoul(pid2, NULL, 16);
				for (z = 0; z < NUM_INPUT_DEVICES; z++) {
					if (valPid == product[z].Id) {
						Com_Printf("Model:            " S_COLOR_GREEN "%s\n", product[z].description);
						break;
					}
				}
					if (z == NUM_INPUT_DEVICES)
						Com_Printf("Model:            " S_COLOR_GREEN "0x%04X\n", valPid);

					switch (devInfo.keyboard.dwType) {
					case 0x51:
						Com_Printf("Type:             " S_COLOR_GREEN "HID keyboard\n");
						break;
					case 0x4:
						Com_Printf("Type:             " S_COLOR_GREEN "Enhanced 101- or 102-key keyboards (and compatibles)\n");
						break;
					case 0x7:
						Com_Printf("Type:             " S_COLOR_GREEN "Japanese Keyboard\n");
						break;
					case 0x8:
						Com_Printf("Type:             " S_COLOR_GREEN "Korean Keyboard\n");
						break;
					}
						Com_Printf("Num keys:         " S_COLOR_GREEN "%i\n", devInfo.keyboard.dwNumberOfKeysTotal);
			}

			break;
		}
	}
	Com_Printf("\n------------------------------------\n");
	free(pRawInputDeviceList);

}

/*
===========
IN_Init
===========
*/
void IN_Init (void) {
	// mouse variables
	m_inversion = Cvar_Get ("m_inversion", "0", CVAR_ARCHIVE);

	// centering
	v_centermove = Cvar_Get ("v_centermove", "0.15", 0);
	v_centerspeed = Cvar_Get ("v_centerspeed", "500", 0);

	IN_PrintRawDevicesList();
	IN_StartupXInput();
}

/*
===========
IN_Shutdown
===========
*/
void IN_Shutdown (void) {
	IN_DeactivateMouse ();
	IN_ShutDownXinput();
}


/*
===========
IN_Activate

Called when the main window gains or loses focus.
The window may have been destroyed and recreated
between a deactivate and an activate.
===========
*/
void IN_Activate (qboolean active) {
	in_appactive = active;
	mouseactive = !active;		// force a new window check or turn off
}


/*
==================
IN_Frame

Called every frame, even if not generating commands
==================
*/
extern int bind_grab;

void IN_Frame (void) {

	if (!in_appactive)
	{
		IN_DeactivateMouse();
		return;
	}

	/*
	if (!cl.refresh_prepped
		|| cls.key_dest == key_console
		|| cls.key_dest == key_menu) {
		// temporarily deactivate if in fullscreen
		if (Cvar_VariableInteger ("r_fullScreen") == 0) {
			IN_DeactivateMouse ();
			return;
		}
	}
	*/
	IN_ActivateMouse ();
}

/*
===========
IN_Move
===========
*/
void IN_Move (usercmd_t *cmd) {
	
	IN_ToggleXInput();

	if (ActiveApp) {
		if (xInputActive) {
			IN_ControllerMove(cmd);
		}
	}
}


/*
===================
IN_ClearStates
===================
*/
void IN_ClearStates (void) {
	mx_accum = 0;
	my_accum = 0;
	mouse_oldbuttonstate = 0;
}