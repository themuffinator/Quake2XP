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
// xBox 360 controller support
// based on DirectQuake by MH

#include "../client/client.h"
#include "winquake.h"
#include "xinput.h"

extern	unsigned	sys_msg_time;

qboolean	xInputActive			= qfalse;
int			xInputActiveController	= -1;
int			xInputOldDpadState		= 0;
int			xInputOldButtonState	= 0;

typedef struct {
	HINSTANCE device;
} xInput_t;

xInput_t xInput;

#define XINPUT_LIB	"xinput1_3.dll" // win7 support

#define XINPUT_MAX_CONTROLLERS 4
#define XINPUT_MAX_CONTROLLER_BUTTONS 10

typedef void	(__stdcall * _xInputEnable)(BOOL);
typedef DWORD	(__stdcall * _XInputGetCapabilities)(DWORD, DWORD, PXINPUT_CAPABILITIES);
typedef DWORD	(__stdcall * _XInputGetState)(DWORD, PXINPUT_STATE);
typedef DWORD	(__stdcall * _XInputGetBatteryInformation)(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);
typedef DWORD	(__stdcall * _XInputSetState)(DWORD, XINPUT_VIBRATION*);

static void		(WINAPI * qXInputEnable)(BOOL enable);
static DWORD	(WINAPI * qXInputGetCapabilities)(DWORD dwUserIndex, DWORD dwFlags, PXINPUT_CAPABILITIES pCapabilities);
static DWORD	(WINAPI * qXInputGetState)(DWORD dwUserIndex, PXINPUT_STATE pState);
static DWORD	(WINAPI * qXInputGetBatteryInformation)(DWORD dwUserIndex, BYTE devType, XINPUT_BATTERY_INFORMATION* pBatteryInformation);
static DWORD	(WINAPI * qXInputSetState)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

void IN_ShutDownXinput() {

	Com_Printf("..." S_COLOR_YELLOW "shutting down xInput subsystem\n");

	if (xInput.device) {
		Com_Printf("..." S_COLOR_YELLOW "unloading " S_COLOR_GREEN "%s\n", XINPUT_LIB);
		FreeLibrary(xInput.device);
	}
	memset(&xInput, 0, sizeof(xInput_t));
}

void IN_StartupXInput(void)
{
	int numDev, firstDev;
	XINPUT_CAPABILITIES xiCaps;
	XINPUT_BATTERY_INFORMATION batteryInfo;
	char batteryLevel[64], batteryType[64];

	// reset to -1 each time as this can be called at runtime
	xInputActiveController = -1;
	xInputActive = qfalse;

	Com_Printf("\n======= Init xInput Devices =======\n\n");

	// Load the xInput dll
	Com_Printf("...calling LoadLibrary(%s): ", XINPUT_LIB);
	if ((xInput.device = LoadLibrary(XINPUT_LIB)) == NULL)
	{
		Com_Printf(S_COLOR_RED"failed!\n");
		Com_Printf("\n-----------------------------------\n\n");
		return;
	}

	qXInputEnable = (_xInputEnable)GetProcAddress(xInput.device, "XInputEnable");
	qXInputGetCapabilities = (_XInputGetCapabilities)GetProcAddress(xInput.device, "XInputGetCapabilities");
	qXInputGetState = (_XInputGetState)GetProcAddress(xInput.device, "XInputGetState");
	qXInputGetBatteryInformation = (_XInputGetBatteryInformation)GetProcAddress(xInput.device, "XInputGetBatteryInformation");
	qXInputSetState = (_XInputSetState)GetProcAddress(xInput.device, "XInputSetState");

	if (!qXInputEnable || !qXInputGetCapabilities || !qXInputGetState || !qXInputGetBatteryInformation || !qXInputSetState)
	{
		Com_Printf(S_COLOR_RED"can't find xInput procedures adresses.\n");
		IN_ShutDownXinput();
		Com_Printf("\n-----------------------------------\n\n");
		return;
	}
	Com_Printf(S_COLOR_GREEN"succeeded.\n\n");

	in_useXInput			= Cvar_Get("in_useXInput", "1", CVAR_ARCHIVE);
	x360_useControllerID	= Cvar_Get("x360_useControllerID", "-1", CVAR_ARCHIVE);
	x360_sensX				= Cvar_Get("x360_sensX", "2.0", CVAR_ARCHIVE);
	x360_sensY				= Cvar_Get("x360_sensY", "0.5", CVAR_ARCHIVE);
	x360_pitchInversion		= Cvar_Get("x360_pitchInversion", "0", CVAR_ARCHIVE);
	x360_swapSticks			= Cvar_Get("x360_swapSticks", "0", CVAR_ARCHIVE);
	x360_swapTriggers		= Cvar_Get("x360_swapTriggers", "0", CVAR_ARCHIVE);

	Com_Printf(S_COLOR_YELLOW"...enumerate xInput Controllers\n\n");
	firstDev = -1;
	for (numDev = 0; numDev < XINPUT_MAX_CONTROLLERS; numDev++)
	{
		memset(&xiCaps, 0, sizeof(XINPUT_CAPABILITIES));
		if (qXInputGetCapabilities(numDev, XINPUT_FLAG_GAMEPAD, &xiCaps) == ERROR_SUCCESS)
		{
			memset(&batteryInfo, 0, sizeof(XINPUT_BATTERY_INFORMATION));
			if (qXInputGetBatteryInformation(numDev, BATTERY_DEVTYPE_GAMEPAD, &batteryInfo) == ERROR_SUCCESS)
			{
				if (batteryInfo.BatteryType == BATTERY_TYPE_WIRED)
					strcpy(batteryType, S_COLOR_YELLOW"...use USB connection\n"S_COLOR_WHITE);
				else if (batteryInfo.BatteryType == BATTERY_TYPE_ALKALINE)
					strcpy(batteryType, S_COLOR_YELLOW"...use Alkalyne battery\n"S_COLOR_WHITE);
				else if (batteryInfo.BatteryType == BATTERY_TYPE_NIMH)
					strcpy(batteryType, S_COLOR_YELLOW"...use Ni-MH battery\n"S_COLOR_WHITE);
				else if (batteryInfo.BatteryType == BATTERY_TYPE_UNKNOWN)
					strcpy(batteryType, S_COLOR_YELLOW"...use unknow battery type\n"S_COLOR_WHITE);

				if (batteryInfo.BatteryLevel == BATTERY_LEVEL_EMPTY)
					strcpy(batteryLevel, S_COLOR_RED"empity"S_COLOR_WHITE);
				else if (batteryInfo.BatteryLevel == BATTERY_LEVEL_LOW)
					strcpy(batteryLevel, S_COLOR_MAGENTA"level low"S_COLOR_WHITE);
				else if (batteryInfo.BatteryLevel == BATTERY_LEVEL_MEDIUM)
					strcpy(batteryLevel, S_COLOR_YELLOW"level medium"S_COLOR_WHITE);
				else if (batteryInfo.BatteryLevel == BATTERY_LEVEL_FULL)
					strcpy(batteryLevel, S_COLOR_GREEN"level full"S_COLOR_WHITE);
				else
					strcpy(batteryLevel, S_COLOR_CYAN"unknown level"S_COLOR_WHITE);

				Com_Printf("Controller " S_COLOR_GREEN "%i" S_COLOR_WHITE ":\n%s<%s>\n", numDev, batteryType, batteryLevel);
			}
				if (firstDev == -1)
					firstDev = numDev;

				// store to global active controller
				if (x360_useControllerID->integer < 0)  /// automatic select
					xInputActiveController = numDev;
				else
				{
					if (x360_useControllerID->integer == numDev)
						xInputActiveController = numDev;
				}
		}
	}

	/// Berserker: если ничего выбралось и если есть хоть один рабочий контроллер, выберем его
	if (xInputActiveController == -1 && firstDev != -1)
		xInputActiveController = firstDev;

	if (xInputActiveController != -1)
	{
		qXInputEnable(TRUE);
		xInputActive = qtrue;
	}
	else
	{
		Com_Printf(S_COLOR_MAGENTA"...xInput Device disconnected or not found.\n");
		IN_ShutDownXinput();
	}

	Com_Printf("\n-----------------------------------\n\n");
}

void IN_ToggleXInput()
{

	if (in_useXInput->integer){
		
		if (xInputActive)
			return;

		if (xInputActiveController != -1) {
			qXInputEnable(TRUE);
			xInputActive = qtrue;
		}
		xInputOldDpadState = xInputOldButtonState = 0;
	}
	else 
	{
		if (!xInputActive)
			return;

		qXInputEnable(FALSE);
		xInputActive = qfalse;
		xInputOldDpadState = xInputOldButtonState = 0;
	}
}

void SetRumble(int devNum, int rumbleLow, int rumbleHigh) {

	if (!xInputActive)
		return;

	if (devNum < 0 || devNum >= XINPUT_MAX_CONTROLLERS)
		return;

	if (!in_useXInput->integer)
		return;

	XINPUT_VIBRATION vibration;
	vibration.wLeftMotorSpeed = clamp(rumbleLow, 0, 65535);
	vibration.wRightMotorSpeed = clamp(rumbleHigh, 0, 65535);
	DWORD err = qXInputSetState(devNum, &vibration);

	if (err != ERROR_SUCCESS)
		Com_Printf(S_COLOR_RED"XInputSetState error: 0x%x", err);
}

extern cvar_t *cl_forwardspeed;
extern cvar_t *cl_sidespeed;

extern cvar_t *cl_yawspeed;
extern cvar_t *cl_pitchspeed;

#define XINPUT_AXIS_NONE		0
#define XINPUT_AXIS_LOOK		1
#define XINPUT_AXIS_MOVE		2
#define XINPUT_AXIS_TURN		3	
#define XINPUT_AXIS_STRAFE		4

#define XINPUT_AXIS_INVLOOK		5
#define XINPUT_AXIS_INVMOVE		6
#define XINPUT_AXIS_INVTURN		7
#define XINPUT_AXIS_INVSTRAFE	8

#define	XINPUT_LEFT_THUMB_X		4
#define XINPUT_LEFT_THUMB_Y		2
#define XINPUT_RIGHT_THUMB_X	3
#define XINPUT_RIGHT_THUMB_Y	1

void IN_ControllerAxisMove(usercmd_t *cmd, int axisval, int dz, int axismax, int type)
{
	// not using this axis
	if (type <= XINPUT_AXIS_NONE)
		return;

	// unimplemented
	if (type > XINPUT_AXIS_INVSTRAFE)
		return;

	// get the amount moved less the deadzone
	int realmove = abs(axisval) - dz;

	// move is within deadzone threshold
	if (realmove < dz)
		return;

	// 0 to 1 scale
	float fmove = (float)realmove / (axismax - dz);

	float speed;
	if ((in_speed.state & 1) ^ cl_run->integer)
		speed = 2;
	else
		speed = 1;

	// square it to get better scale at small moves
	fmove *= fmove;

	// go back to negative
	if (axisval < 0) 
		fmove *= -1;

	// check for inverse scale
	if (type > XINPUT_AXIS_STRAFE)
		fmove *= -1;
	
	float inv = 1;
	if(x360_pitchInversion->value)
		inv *= -1;

	// decode the move
	switch ( type )
	{
	case XINPUT_AXIS_LOOK:
	case XINPUT_AXIS_INVLOOK:
		cl.viewangles_PITCH -= fmove * (cl_pitchspeed->value / cl.refdef.fov_y) * x360_sensY->value * inv;
		break;

	case XINPUT_AXIS_MOVE:
	case XINPUT_AXIS_INVMOVE:
		cmd->forwardmove += fmove * speed * cl_forwardspeed->value;
		break;

	case XINPUT_AXIS_TURN:
	case XINPUT_AXIS_INVTURN:
		// slow this down because the default cl_yawspeed is too fast here
		// invert it so that positive move = right
		cl.viewangles_YAW -= fmove * (cl_yawspeed->value / cl.refdef.fov_x) * x360_sensX->value;
		break;

	case XINPUT_AXIS_STRAFE:
	case XINPUT_AXIS_INVSTRAFE:
		cmd->sidemove = fmove * speed * cl_sidespeed->value;
		break;

	default:
		// unimplemented
		break;
	}
}


void IN_ZoomDown(void);
void IN_ZoomUp(void);
void IN_AttackDown(void);
void IN_AttackUp(void);

void IN_ControllerMove(usercmd_t *cmd)
{
	// no controller to use
	if (!xInputActive)
		return;

	if (xInputActiveController < 0)
		return;

	if (!in_useXInput->integer)
		return;

	XINPUT_STATE xInputStage;
	static DWORD xInputLastPacket = 666;

	// get current state
	DWORD xInputResult = qXInputGetState(xInputActiveController, &xInputStage);

	if (xInputResult != ERROR_SUCCESS)
		return;

	if (!x360_swapSticks->integer) {
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,	32768,	XINPUT_LEFT_THUMB_X);
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,	32768,	XINPUT_LEFT_THUMB_Y);
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,	32768,	XINPUT_RIGHT_THUMB_X);
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,	32768,	XINPUT_RIGHT_THUMB_Y);
	}
	else {
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,	32768,	XINPUT_RIGHT_THUMB_X);
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE,	32768,	XINPUT_RIGHT_THUMB_Y);
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,	32768,	XINPUT_LEFT_THUMB_X);
		IN_ControllerAxisMove(cmd, xInputStage.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE,	32768,	XINPUT_LEFT_THUMB_Y);
	}

	// fix up the command (bound/etc)
	if (cl.viewangles[0] > 80.0) 
		cl.viewangles[0] = 80.0;

	if (cl.viewangles[0] < -70.0) 
		cl.viewangles[0] = -70.0;

	// check for a change of state
	if (xInputLastPacket == xInputStage.dwPacketNumber)
		return;

	// store back last packet
	xInputLastPacket = xInputStage.dwPacketNumber;

	int buttonState = 0;
	int dpadState = 0;

	// Hardcoded!!!
	if (!x360_swapTriggers->integer) {
		if (xInputStage.Gamepad.bLeftTrigger >= 128)
			IN_ZoomDown();
		else
			IN_ZoomUp();

		if (xInputStage.Gamepad.bRightTrigger >= 128)
			IN_AttackDown();
		else
			IN_AttackUp();
	}
	else {

		if (xInputStage.Gamepad.bRightTrigger >= 128)
			IN_ZoomDown();
		else
			IN_ZoomUp();

		if (xInputStage.Gamepad.bLeftTrigger >= 128)
			IN_AttackDown();
		else
			IN_AttackUp();
	}

	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)		dpadState |= 1;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)	dpadState |= 2;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)	dpadState |= 4;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)	dpadState |= 8;

	// check for event changes
	for (int i = 0; i < XINPUT_MAX_CONTROLLERS; i++){

		if ((dpadState & (1 << i)) && !(xInputOldDpadState & (1 << i)))
			Key_Event(K_UPARROW + i, qtrue, sys_msg_time);

		if (!(dpadState & (1 << i)) && (xInputOldDpadState & (1 << i)))
			Key_Event(K_UPARROW + i, qfalse, sys_msg_time);
	}

	// store back
	xInputOldDpadState = dpadState;

	// check other buttons - map these to K_JOY buttons
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_START)			buttonState |= 1;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)				buttonState |= 2;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB)		buttonState |= 4;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB)		buttonState |= 8;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)	buttonState |= 16;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)	buttonState |= 32;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_A)				buttonState |= 64;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_B)				buttonState |= 128;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_X)				buttonState |= 256;
	if (xInputStage.Gamepad.wButtons & XINPUT_GAMEPAD_Y)				buttonState |= 512;

	// check for event changes
	for (int i = 0; i < XINPUT_MAX_CONTROLLER_BUTTONS; i++)
	{
		if ((buttonState & (1 << i)) && !(xInputOldButtonState & (1 << i)))
			Key_Event(K_JOY1 + i, qtrue, sys_msg_time);

		if (!(buttonState & (1 << i)) && (xInputOldButtonState & (1 << i)))
			Key_Event(K_JOY1 + i, qfalse, sys_msg_time);
	}

	// store back
	xInputOldButtonState = buttonState;
}