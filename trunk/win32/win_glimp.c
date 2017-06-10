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
/*
** GLW_IMP.C
**
** This file contains ALL Win32 specific stuff having to do with the
** OpenGL refresh.  When a port is being made the following functions
** must be implemented by the port:
**
** GLimp_EndFrame
** GLimp_Init
** GLimp_Shutdown
** GLimp_SwitchFullscreen
**
*/

#include "../ref_gl/r_local.h"
#include "nvapi/nvapi.h"

// Enable High Performance Graphics while using Integrated Graphics.
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;        // Nvidia
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;  // AMD

qboolean nvApiInit;

#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE)

typedef struct {
	qboolean		accelerated;
	qboolean		drawToWindow;
	qboolean		supportOpenGL;
	qboolean		doubleBuffer;
	qboolean		rgba;

	int				colorBits;
	int				alphaBits;
	int				depthBits;
	int				stencilBits;
	int				samples;
} glwPixelFormatDescriptor_t;

static LRESULT CALLBACK FakeWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, msg, wParam, lParam);
}
#define	WINDOW_CLASS_FAKE		"quake2xp Fake Window"
#define	WINDOW_NAME				"quake2xp"

qboolean GLW_InitDriver(void);

glwstate_t glw_state;

/*
** VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"quake2xp"

qboolean VID_CreateWindow( int width, int height, qboolean fullscreen )
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos;
	int				stylebits;
	int				x, y;
	int				exstyle;
	DEVMODE			dm;

	/* Register the frame class */
    wc.style         = 0;
    wc.lpfnWndProc   = (WNDPROC)glw_state.wndproc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = glw_state.hInstance;
    wc.hIcon         = 0;
    wc.hCursor       = LoadCursor (NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
    wc.lpszMenuName  = 0;
    wc.lpszClassName = WINDOW_CLASS_NAME;

    if (!RegisterClass (&wc) )
		VID_Error (ERR_FATAL, "Couldn't register window class");

	// compute width and height
	memset(&dm, 0, sizeof(dm));
	dm.dmSize = sizeof(dm);
	if (glw_state.desktopName[0])
	{
		if (!EnumDisplaySettings(glw_state.desktopName, ENUM_CURRENT_SETTINGS, &dm))
		{
			memset(&dm, 0, sizeof(dm));
			dm.dmSize = sizeof(dm);
		}
	}
	/// save real monitor position in the virtual monitor
	glw_state.desktopPosX = dm.dmPosition.x;
	glw_state.desktopPosY = dm.dmPosition.y;

	if (fullscreen)
	{
		exstyle = WS_EX_TOPMOST;
		stylebits = WS_POPUP|WS_VISIBLE;
	}
	else
	{
		exstyle = 0;
		stylebits = WINDOW_STYLE;
	}

	r.left = glw_state.desktopPosX;
	r.top = glw_state.desktopPosY;
	r.right = width + glw_state.desktopPosX;
	r.bottom = height + glw_state.desktopPosY;
	
	glw_state.virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	glw_state.virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	glw_state.virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	glw_state.virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	glw_state.borderWidth = GetSystemMetrics(SM_CXBORDER) * 3;
	glw_state.borderHeight = GetSystemMetrics(SM_CYBORDER) * 3 + GetSystemMetrics(SM_CYCAPTION);
	vid_xpos = Cvar_Get("vid_xpos", "0", 0);
	vid_ypos = Cvar_Get("vid_ypos", "0", 0);

	AdjustWindowRect (&r, stylebits, FALSE);

	if (fullscreen)
	{
		x = glw_state.desktopPosX;
		y = glw_state.desktopPosY;
	}
	else
	{
		x = vid_xpos->value;
		y = vid_ypos->value;

		// adjust window coordinates if necessary
		// so that the window is completely on screen
		if (x < glw_state.virtualX)
			x = glw_state.virtualX;
		if (x > glw_state.virtualX + glw_state.virtualWidth - 64)
			x = glw_state.virtualX + glw_state.virtualWidth - 64;
		if (y < glw_state.virtualY)
			y = glw_state.virtualY;
		if (y > glw_state.virtualY + glw_state.virtualHeight - 64)
			y = glw_state.virtualY + glw_state.virtualHeight - 64;
	}

	glw_state.hWnd = CreateWindowEx (
		 exstyle, 
		 WINDOW_CLASS_NAME,
		 "quake2xp",
		 stylebits,
		 x, y, width, height,
		 NULL,
		 NULL,
		 glw_state.hInstance,
		 NULL);

	if (!glw_state.hWnd)
		VID_Error (ERR_FATAL, "Couldn't create window");

	ShowWindow( glw_state.hWnd, SW_SHOW );
	UpdateWindow( glw_state.hWnd );

	// init all the gl stuff for the window
	if (!GLW_InitDriver())
	{
		Com_Printf(S_COLOR_RED"...destroying window\n");
		Com_Printf(S_COLOR_RED "VID_CreateWindow() - GLimp_InitGL failed\n");

		ShowWindow(glw_state.hWnd, SW_HIDE);
		DestroyWindow(glw_state.hWnd);
		glw_state.hWnd = NULL;

		UnregisterClass(WINDOW_CLASS_NAME, glw_state.hInstance);
		return qfalse;
	}

	SetForegroundWindow( glw_state.hWnd );
	SetFocus( glw_state.hWnd );

	// let the sound and input subsystems know about the new window
	VID_NewWindow (width, height);

	return qtrue;
}

#define MAX_SUPPORTED_MONITORS  16
int monitorCounter;
MONITORINFO monitorInfos[MAX_SUPPORTED_MONITORS];
char        monitorNames[MAX_SUPPORTED_MONITORS][16];

BOOL GetDisplayMonitorInfo(char *monitorName, char *monitorModel)
{
	DISPLAY_DEVICE  dd;
	int             i = 0;
	BOOL            bRet = FALSE;

	monitorModel[0] = 0;
	ZeroMemory(&dd, sizeof(dd));
	dd.cb = sizeof(dd);

	while (EnumDisplayDevices(glw_state.desktopName, i, &dd, EDD_GET_DEVICE_INTERFACE_NAME))
	{
		if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE)
		{
			char *p, *s;
			char deviceID[128];
			char regPath[128];
			byte edid[128];
			HKEY hKey;
			int j = 0;
			lstrcpy(deviceID, dd.DeviceID);
			p = strstr(deviceID, "DISPLAY");
			if (p)
			{
				s = p;
				while (1)
				{
					if (*s == 0)
					{
						j = -1; // not found
						break;
					}
					if (*s == '#')
					{
						j++;
						if (j == 3)
						{
							*s = 0;
							break;
						}
						else
							*s = '\\';
					}
					s++;
				}
				if (j != -1)
				{
					LSTATUS err;
					Com_sprintf(regPath, sizeof(regPath), "SYSTEM\\CurrentControlSet\\Enum\\%s\\Device Parameters\\", p);
					err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey);
					if (err == ERROR_SUCCESS)
					{
						DWORD buflen = sizeof(edid);
						err = RegQueryValueEx(hKey, "EDID", NULL, NULL, edid, &buflen);
						RegCloseKey(hKey);
						if (err == ERROR_SUCCESS)
						{
							int k, m, n, descOffs[4] = { 54, 72, 90, 108 };
							for (k = 0; k < 4; k++)
							{
								byte *desc = &edid[descOffs[k]];
								if (desc[0] == 0 && desc[1] == 0 && desc[2] == 0 && desc[3] == 0xFC)
								{
									Q_strncpyz(monitorModel, &desc[5], 13);
									n = strlen(monitorModel);
									for (m = 0; m < n; m++)
										if (monitorModel[m] == '\n')
											monitorModel[m] = 0;
									break;
								}
							}
						}
					}
				}
			}

			lstrcpy(monitorName, dd.DeviceString);
			bRet = TRUE;
			break;
		}
		i++;
	}

	return bRet;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	monitorInfos[monitorCounter].cbSize = sizeof(monitorInfos[monitorCounter]);
	if (GetMonitorInfo(hMonitor, &monitorInfos[monitorCounter]))
	{
		monitorCounter++;
		if (monitorCounter == MAX_SUPPORTED_MONITORS)
			return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK MonitorEnumProc2(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	monitorInfos[monitorCounter].cbSize = sizeof(monitorInfos[monitorCounter]);
	if (GetMonitorInfo(hMonitor, &monitorInfos[monitorCounter]))
	{
		Com_Printf("   " S_COLOR_GREEN "%i" S_COLOR_WHITE ": %i " S_COLOR_GREEN "x" S_COLOR_WHITE " %i", monitorCounter + 1,
			abs(monitorInfos[monitorCounter].rcMonitor.left - monitorInfos[monitorCounter].rcMonitor.right),
			abs(monitorInfos[monitorCounter].rcMonitor.top - monitorInfos[monitorCounter].rcMonitor.bottom));
		if (monitorNames[monitorCounter][0])
			Com_Printf(S_COLOR_YELLOW" %s", monitorNames[monitorCounter]);
		else
			Com_Printf(S_COLOR_YELLOW" Unknown model");
		if (monitorInfos[monitorCounter].dwFlags & MONITORINFOF_PRIMARY)
			Com_Printf(" (" S_COLOR_YELLOW "primary" S_COLOR_WHITE ")");
		Com_Printf("\n");
		monitorCounter++;
		if (monitorCounter == MAX_SUPPORTED_MONITORS)
			return FALSE;
	}
	return TRUE;
}
void GLimp_InitADL();
void adl_PrintGpuPerformance();
extern qboolean adlInit;

NvPhysicalGpuHandle hPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS];
NvU32 physicalGpuCount = 0;

char *GLimp_NvApi_GetThermalController(NV_THERMAL_CONTROLLER tc)
{
	switch (tc)
	{
	case NVAPI_THERMAL_CONTROLLER_NONE: return "None";
	case NVAPI_THERMAL_CONTROLLER_GPU_INTERNAL: return "GPU Internal";
	case NVAPI_THERMAL_CONTROLLER_ADM1032: return "ADM1032";
	case NVAPI_THERMAL_CONTROLLER_MAX6649: return "MAX6649";
	case NVAPI_THERMAL_CONTROLLER_MAX1617: return "MAX1617";
	case NVAPI_THERMAL_CONTROLLER_LM99: return "LM99";
	case NVAPI_THERMAL_CONTROLLER_LM89: return "LM89";
	case NVAPI_THERMAL_CONTROLLER_LM64: return "LM64";
	case NVAPI_THERMAL_CONTROLLER_ADT7473: return "ADT7473";
	case NVAPI_THERMAL_CONTROLLER_SBMAX6649: return "SBMAX6649";
	case NVAPI_THERMAL_CONTROLLER_VBIOSEVT: return "VBIOSEVT";
	case NVAPI_THERMAL_CONTROLLER_OS: return "OS";
	default:
	case NVAPI_THERMAL_CONTROLLER_UNKNOWN: return "Unknown";
	}
}

void GLimp_InitNvApi() {

	NvAPI_Status ret = NVAPI_OK;
	NvAPI_ShortString ver, string;

	nvApiInit = qfalse;

	Com_Printf("" S_COLOR_YELLOW "...Initializing NVIDIA API\n\n");
	
	// init nvapi
	ret = NvAPI_Initialize();
	
	if (ret != NVAPI_OK) { // check for nvapi error
		Com_Printf(S_COLOR_MAGENTA"...not supported\n");
		Com_Printf("\n==================================\n");
		return;
	}

	NvAPI_GetInterfaceVersionString(ver);
	Com_Printf("...use" S_COLOR_GREEN " %s\n", ver);

	// Enumerate the physical GPU handle
	ret = NvAPI_EnumPhysicalGPUs(hPhysicalGpu, &physicalGpuCount);

	if (ret != NVAPI_OK) {
		NvAPI_GetErrorMessage(ret, string);
		Com_Printf(S_COLOR_RED"...NvAPI_EnumPhysicalGPUs() fail: %s\n", string);
		return;
	}

	Com_Printf("...found " S_COLOR_GREEN "%i " S_COLOR_WHITE "physical gpu's\n", physicalGpuCount);

	nvApiInit = qtrue;

	Com_Printf("\n==================================\n\n");
}

#define NV_UTIL_DOMAIN_GPU  0
#define NV_UTIL_DOMAIN_FB   1
#define NV_UTIL_DOMAIN_VID  2 //video decoder don't needed
#define NV_UTIL_DOMAIN_BUS  3

void R_NvApi_f(void) {

	NvAPI_Status					ret = NVAPI_OK;
	NvU32							rpm = 0;
	NvAPI_ShortString				string;
	NV_GPU_THERMAL_SETTINGS			thermal;
	NV_GPU_DYNAMIC_PSTATES_INFO_EX	m_DynamicPStateInfo;
	NV_GPU_CLOCK_FREQUENCIES		clocks;
	
	if (adlInit) {
		adl_PrintGpuPerformance();
		return;
	}

	if (!nvApiInit) {
		Com_Printf(S_COLOR_RED"NVAPI not found!\n");
		return;
	}

	Com_Printf("\n==========================================================\n");
	ret = NvAPI_GPU_GetFullName(hPhysicalGpu[0], string);
	
	if (ret != NVAPI_OK) {
		NvAPI_GetErrorMessage(ret, string);
		Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetFullName() fail: %\n", string);
	}
	else
		Com_Printf(S_COLOR_YELLOW"...Get GPU statistic from " S_COLOR_GREEN "%s\n", string);

	for (int i = 0; i < physicalGpuCount; i++) {

		Com_Printf("\n   GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ":\n", i);

		// get gpu temperature
		thermal.version = NV_GPU_THERMAL_SETTINGS_VER_2;
		ret = NvAPI_GPU_GetThermalSettings(hPhysicalGpu[i], 0, &thermal);

		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetThermalSettings() fail: %\n", string);
		}
		else
			Com_Printf("...temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius (%s)\n",	thermal.sensor[0].currentTemp, 
																								GLimp_NvApi_GetThermalController(thermal.sensor[0].controller));

		// get fans speed
		ret = NvAPI_GPU_GetTachReading(hPhysicalGpu[i], &rpm);

		if (ret != NVAPI_OK)
		{
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetTachReading() fail: %s\n", string);
		}
		else
			Com_Printf("...fan speed: " S_COLOR_GREEN "%u" S_COLOR_WHITE " rpm\n", rpm);

		m_DynamicPStateInfo.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
		ret = NvAPI_GPU_GetDynamicPstatesInfoEx(hPhysicalGpu[i], &m_DynamicPStateInfo);
		if (ret != NVAPI_OK)
		{
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetDynamicPstatesInfoEx() fail: %s", string);
		}
		else
			Com_Printf("...utilization: " S_COLOR_YELLOW "Core " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%, " S_COLOR_YELLOW "Frame Buffer " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%, " S_COLOR_YELLOW "PCIe Bus " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%\n",
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_GPU].percentage, 
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_FB].percentage, 
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_BUS].percentage);

		// get gpu & vram frequencies
		clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER;
		clocks.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
		ret = NvAPI_GPU_GetAllClockFrequencies(hPhysicalGpu[i], &clocks);
		
		if (ret != NVAPI_OK){
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetAllClockFrequencies() fail: %s\n", string);
		}
		else {
			if (clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent && clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent)
				Com_Printf("...frequencies: " S_COLOR_YELLOW "GPU: " S_COLOR_GREEN "%u" S_COLOR_WHITE " MHz " S_COLOR_YELLOW "VRAM: " S_COLOR_GREEN "%u" S_COLOR_WHITE " MHz\n",
				(NvU32)((clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency + 500) / 1000),
				(NvU32)((clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency + 500) / 1000));
		}
			

		Com_Printf("\n==========================================================\n");
	}
}


/*
** GLimp_SetMode
*/

rserr_t GLimp_SetMode( unsigned *pwidth, unsigned *pheight, int mode, qboolean fullscreen )
{
	int width, height, i, idx, cvm, cdsRet, j;
	const char *win_fs[] = { "Window", "Full Screen" };
	cvar_t	*vid_monitor = Cvar_Get("vid_monitor", "0", CVAR_ARCHIVE);
	char	monitorName[128], monitorModel[16];
	HDC		hDC;
	DEVMODE dm;

	GLimp_InitNvApi();
	GLimp_InitADL();
	
	Com_Printf("\n==================================\n\n");

	Com_Printf(S_COLOR_YELLOW"...Initializing OpenGL display\n");
	
	Com_Printf("\n==================================\n");

	monitorCounter = 0;
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

	monitorName[0] = 0;
	if ((int)vid_monitor->value <= 0 || (int)vid_monitor->value > MAX_SUPPORTED_MONITORS)
		cvm = 0;    // лишь бы что-то было корректное...
	else
		cvm = (int)vid_monitor->value - 1;
	idx = -1;
	for (j = 0; j < MAX_SUPPORTED_MONITORS; j++)
	{
		monitorNames[j][0] = 0;
		for (i = 1; i < 256; i++)   // много?
		{
			Com_sprintf(glw_state.desktopName, sizeof(glw_state.desktopName), "\\\\.\\Display%i", i);
			memset(&dm, 0, sizeof(dm));
			dm.dmSize = sizeof(dm);
			if (EnumDisplaySettings(glw_state.desktopName, ENUM_CURRENT_SETTINGS, &dm))
			{
				char    tempMonitorName[128];
				glw_state.desktopPosX = dm.dmPosition.x;
				glw_state.desktopPosY = dm.dmPosition.y;
				if (GetDisplayMonitorInfo(tempMonitorName, monitorModel))
				{
					hDC = CreateDC(glw_state.desktopName, tempMonitorName, NULL, NULL);
					if (hDC)
					{   /// monitor found, so compare positions in virtual desktop
						glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
						glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
						if (monitorInfos[j].rcMonitor.left == glw_state.desktopPosX &&
							monitorInfos[j].rcMonitor.top == glw_state.desktopPosY &&
							abs(monitorInfos[j].rcMonitor.left - monitorInfos[j].rcMonitor.right) == glw_state.desktopWidth &&
							abs(monitorInfos[j].rcMonitor.top - monitorInfos[j].rcMonitor.bottom) == glw_state.desktopHeight)
						{
							lstrcpy(monitorNames[j], monitorModel);
							if (j == cvm)
							{
								lstrcpy(monitorName, tempMonitorName);
								idx = i;
								i = 256;    /// break
							}
						}
					}
					DeleteDC(hDC);
				}
			}
		}
	}
	if (idx == -1 || (int)vid_monitor->value <= 0 || (int)vid_monitor->value > MAX_SUPPORTED_MONITORS)    /// not found :(
	{
		glw_state.desktopName[0] = 0;
		Com_Printf("\n...using " S_COLOR_YELLOW "primary" S_COLOR_WHITE " monitor\n");
		glw_state.desktopPosX = 0;
		glw_state.desktopPosY = 0;
		hDC = GetDC(GetDesktopWindow());
		glw_state.desktopBitPixel = GetDeviceCaps(hDC, BITSPIXEL);
		glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
		glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
		ReleaseDC(GetDesktopWindow(), hDC);
	}
	else
	{
		Com_sprintf(glw_state.desktopName, sizeof(glw_state.desktopName), "\\\\.\\Display%i", idx);
		Com_Printf("\n...calling " S_COLOR_YELLOW "CreateDC" S_COLOR_WHITE "('" S_COLOR_GREEN "%s" S_COLOR_WHITE "','" S_COLOR_GREEN "%s" S_COLOR_WHITE "')\n", glw_state.desktopName, monitorName);
		memset(&dm, 0, sizeof(dm));
		dm.dmSize = sizeof(dm);
		EnumDisplaySettings(glw_state.desktopName, ENUM_CURRENT_SETTINGS, &dm);
		glw_state.desktopPosX = dm.dmPosition.x;
		glw_state.desktopPosY = dm.dmPosition.y;
		hDC = CreateDC(glw_state.desktopName, monitorName, NULL, NULL);
		glw_state.desktopBitPixel = GetDeviceCaps(hDC, BITSPIXEL);
		glw_state.desktopWidth = GetDeviceCaps(hDC, HORZRES);
		glw_state.desktopHeight = GetDeviceCaps(hDC, VERTRES);
		DeleteDC(hDC);

		if (monitorNames[cvm][0])
			Com_Printf("...using monitor " S_COLOR_GREEN "%i " S_COLOR_WHITE "(" S_COLOR_GREEN "%s" S_COLOR_WHITE ")\n", (int)vid_monitor->value, monitorNames[cvm]);
		else
			Com_Printf("...using monitor " S_COLOR_GREEN "%i\n", (int)vid_monitor->value);

	}
		Com_Printf(S_COLOR_YELLOW"\n...Available monitors:\n\n");
		monitorCounter = 0;
		EnumDisplayMonitors(NULL, NULL, MonitorEnumProc2, 0);

	
	Com_Printf("\n==================================\n\n");

	if ( !VID_GetModeInfo( &width, &height, mode ) )
	{
		Com_Printf(S_COLOR_RED " invalid mode\n" );
		return rserr_invalid_mode;
	}
	
	if(mode == 0){
	width = glw_state.desktopWidth;
	height = glw_state.desktopHeight;
	}

	Com_Printf ("...setting mode "S_COLOR_YELLOW"%d"S_COLOR_WHITE":"S_COLOR_YELLOW"[%ix%i]", mode , width, height);

	if(width > glw_state.desktopWidth || height > glw_state.desktopHeight){
		width = glw_state.desktopWidth;
		height = glw_state.desktopHeight;
		Com_Printf(S_COLOR_RED "\n!!!Invalid Resolution!!!\n"S_COLOR_MAGENTA"Set Current Desktop Resolution\n"S_COLOR_WHITE"%i"S_COLOR_GREEN"x"S_COLOR_WHITE"%i "S_COLOR_WHITE"%s\n", 
					width, height, win_fs[fullscreen]);
		
	} else
	Con_Printf( PRINT_ALL, " "S_COLOR_WHITE"%s\n", win_fs[fullscreen] );

	// destroy the existing window
	if (glw_state.hWnd)
	{
		GLimp_Shutdown ();
	}

	// do a CDS if needed
	if ( fullscreen )
	{
		Com_Printf("...attempting fullscreen\n" );

		memset( &dm, 0, sizeof( dm ) );
		dm.dmSize = sizeof( dm );

		dm.dmPelsWidth  = width;
		dm.dmPelsHeight = height;
		dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
 
		/* display frequency */
		if (r_displayRefresh->value != 0){
	        gl_state.displayrefresh	= r_displayRefresh->value;
			dm.dmDisplayFrequency	= r_displayRefresh->value;
			dm.dmFields				|= DM_DISPLAYFREQUENCY;
			Com_Printf("...display frequency is "S_COLOR_GREEN"%d"S_COLOR_WHITE" hz\n", gl_state.displayrefresh);
		}
		else {
			
			int displayref = GetDeviceCaps (hDC, VREFRESH);
            dm.dmDisplayFrequency	= displayref;
			dm.dmFields				|= DM_DISPLAYFREQUENCY;
			Com_Printf("...using desktop frequency.\n");
		}
     
			// force set 32-bit color depth
			dm.dmBitsPerPel = 32;
			dm.dmFields |= DM_BITSPERPEL;

			
		Con_Printf( PRINT_ALL, "...calling CDS: " );
		
		if (glw_state.desktopName[0])
			cdsRet = ChangeDisplaySettingsEx(glw_state.desktopName, &dm, NULL, CDS_FULLSCREEN, NULL);
		else
			cdsRet = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

		if (cdsRet == DISP_CHANGE_SUCCESSFUL )
		{
			*pwidth = width;
			*pheight = height;

			gl_state.fullscreen = qtrue;

			Com_Printf(S_COLOR_GREEN"ok\n" );

			if ( !VID_CreateWindow (width, height, qtrue) )
				return rserr_invalid_mode;

			return rserr_ok;
		}
		else
		{
			*pwidth = width;
			*pheight = height;

			Com_Printf(S_COLOR_RED"failed\n" );

			Com_Printf("...calling CDS assuming dual monitors:" );

			dm.dmPelsWidth = width * 2;
			dm.dmPelsHeight = height;
			dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
		
			/*
			** our first CDS failed, so maybe we're running on some weird dual monitor
			** system 
			*/
			if (glw_state.desktopName[0])
				cdsRet = ChangeDisplaySettingsEx(glw_state.desktopName, &dm, NULL, CDS_FULLSCREEN, NULL);
			else
				cdsRet = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

			if (cdsRet != DISP_CHANGE_SUCCESSFUL )
			{
				Com_Printf(S_COLOR_RED" failed\n" );

				Com_Printf(S_COLOR_YELLOW"...setting windowed mode\n" );

				ChangeDisplaySettings( 0, 0 );

				*pwidth = width;
				*pheight = height;
				gl_state.fullscreen = qfalse;
				if ( !VID_CreateWindow (width, height, qfalse) )
					return rserr_invalid_mode;
				return rserr_invalid_fullscreen;
			}
			else
			{
				Com_Printf(S_COLOR_GREEN" ok\n" );
				if ( !VID_CreateWindow (width, height, qtrue) )
					return rserr_invalid_mode;
				gl_state.fullscreen = qtrue;
				return rserr_ok;
			}
		}
	}
	else
	{
		Com_Printf("...setting windowed mode\n" );

		ChangeDisplaySettings( 0, 0 );

		*pwidth = width;
		*pheight = height;
		gl_state.fullscreen = qfalse;
		if ( !VID_CreateWindow (width, height, qfalse) )
			return rserr_invalid_mode;
	}
	return rserr_ok;
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void ADL_unload();

void GLimp_Shutdown( void )
{

	if ( qwglMakeCurrent && !qwglMakeCurrent( NULL, NULL ) )
		Com_Printf(S_COLOR_RED"ref_gl::R_Shutdown() - wglMakeCurrent failed\n");
	if ( glw_state.hGLRC )
	{
		if (  qwglDeleteContext && !qwglDeleteContext( glw_state.hGLRC ) )
			Com_Printf(S_COLOR_RED"ref_gl::R_Shutdown() - wglDeleteContext failed\n");
		glw_state.hGLRC = NULL;
	}
	if (glw_state.hDC)
	{
		if ( !ReleaseDC( glw_state.hWnd, glw_state.hDC ) )
			Com_Printf(S_COLOR_RED"ref_gl::R_Shutdown() - ReleaseDC failed\n" );
		glw_state.hDC   = NULL;
	}
	if (glw_state.hWnd)
	{
		DestroyWindow (	glw_state.hWnd );
		glw_state.hWnd = NULL;
	}


	UnregisterClass (WINDOW_CLASS_NAME, glw_state.hInstance);

	if ( gl_state.fullscreen )
	{
		ChangeDisplaySettings( 0, 0 );
		gl_state.fullscreen = qfalse;
	}
	
	if(nvApiInit)
		NvAPI_Unload();

	if(adlInit)
		ADL_unload();
}

/*=============
CPU DETECTION
=============*/

int MeasureCpuSpeed()
    {
	unsigned __int64	start, end, counter, stop, frequency;
	uint speed;

	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);

		__asm {
			rdtsc
			mov dword ptr[start+0], eax
			mov dword ptr[start+4], edx
		}

		QueryPerformanceCounter((LARGE_INTEGER *)&stop);
		stop += frequency;

		do {
			QueryPerformanceCounter((LARGE_INTEGER *)&counter);
		} while (counter < stop);

		__asm {
			rdtsc
			mov dword ptr[end+0], eax
			mov dword ptr[end+4], edx
		}

		speed = (unsigned)((end - start) / 1000000);
		return speed;

    }


qboolean GetCpuCoresThreads( void ) {
	char cpuVendor[12];
	uint regs[4], cpuFeatures, cores, logical;
	qboolean hyperThreads;

	// Get vendor
	__cpuid(regs, 0);
	((uint *)cpuVendor)[0] = regs[1];   // EBX
	((uint *)cpuVendor)[1] = regs[3];   // EDX
	((uint *)cpuVendor)[2] = regs[2];   // ECX

	// Get CPU features
	__cpuid(regs, 1);
	cpuFeatures = regs[3];  // EDX

	// Logical core count per CPU
	logical = (regs[1] >> 16) & 0xff;       // EBX[23:16]

	if (!strncmp(cpuVendor, "GenuineIntel", 12))
	{
		// Get DCP cache info
		__cpuid(regs, 4);
		cores = ((regs[0] >> 26) & 0x3f) + 1;   // EAX[31:26] + 1
	}
	else if (!strncmp(cpuVendor, "AuthenticAMD", 12))
	{
		// Get NC: Number of CPU cores - 1
		__cpuid(regs, 0x80000008);
		cores = ((uint)(regs[2] & 0xff)) + 1;       // ECX[7:0] + 1
	}

	// Detect hyper-threads  
	hyperThreads = cpuFeatures & (1 << 28) && cores < logical;

	if (hyperThreads)
	{
		Com_Printf("Cores/Threads: "S_COLOR_GREEN"%i"S_COLOR_WHITE"|"S_COLOR_GREEN"%i\n", cores >> 1, logical >> 1);
		return qtrue;
	}
	else
	{
		Com_Printf("Cores/Threads: "S_COLOR_GREEN"%i"S_COLOR_WHITE"|"S_COLOR_GREEN"%i\n", cores, logical);
		return qfalse;
	}
}

void GLimp_CpuID(void)
{
    char		CPUString[0x20];
    char		CPUBrandString[0x40];
    int			CPUInfo[4]		= {-1};
    int			nFeatureInfo	= 0;
    uint	    nIds, nExIds, i;
	uint		dwCPUSpeed = MeasureCpuSpeed();

	qboolean    SSE3	= qfalse;
	qboolean	SSE4	= qfalse;
	qboolean	SSE2	= qfalse;
	qboolean	SSE		= qfalse;
	qboolean	MMX		= qfalse;
	qboolean	HTT		= qfalse;
	qboolean	EM64T	= qfalse;

    // __cpuid with an InfoType argument of 0 returns the number of
    // valid Ids in CPUInfo[0] and the CPU identification string in
    // the other three array elements. The CPU identification string is
    // not in linear order. The code below arranges the information 
    // in a human readable form.
    __cpuid(CPUInfo, 0);
	nIds = CPUInfo[0];
	memset(CPUString, 0, sizeof(CPUString));

    *((int*)CPUString)		= CPUInfo[1];
    *((int*)(CPUString+4))	= CPUInfo[3];
    *((int*)(CPUString+8))	= CPUInfo[2];
	
    // Get the information associated with each valid Id
    for (i=0; i<=nIds; ++i){

        __cpuid(CPUInfo, i);
   
        // Interpret CPU feature information.
        if  (i == 1)
        {
            SSE4					= (CPUInfo[2] & BIT (9));
            SSE3					= (CPUInfo[2] & BIT (0));
			SSE2					= (CPUInfo[3] & BIT(26));
			SSE						= (CPUInfo[3] & BIT(25));
			MMX						= (CPUInfo[3] & BIT(23));
			EM64T					= (CPUInfo[3] & BIT(29));
            nFeatureInfo			=  CPUInfo[3];
        }
    }
    // Calling __cpuid with 0x80000000 as the InfoType argument
    // gets the number of valid extended IDs.
    __cpuid(CPUInfo, 0x80000000);
    nExIds = CPUInfo[0];
    memset(CPUBrandString, 0, sizeof(CPUBrandString));

    // Get the information associated with each extended ID.
    for (i=0x80000000; i<=nExIds; ++i){

        __cpuid(CPUInfo, i);
  
        // Interpret CPU brand string.
        if  (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else 
			if  (i == 0x80000003)
					memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else 
			if  (i == 0x80000004)
					memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));       	
	 }

    if  (nIds >= 1){
 
        if(nFeatureInfo || SSE3 || MMX || SSE || SSE2 || SSE4 || EM64T){
       		
		Com_Printf ("Cpu Brand Name: "S_COLOR_GREEN"%s\n", &CPUBrandString[0]);
		HTT = GetCpuCoresThreads();
		float GHz = (float)dwCPUSpeed * 0.001;
		Com_Printf ("CPU Speed: ~"S_COLOR_GREEN"%.3f"S_COLOR_WHITE" GHz\n", GHz);
		Com_Printf ("Supported Extensions: ");
				
		__cpuid(CPUInfo, 0x80000001);
		
			if(CPUInfo[3] & 0x80000000)
				Com_Printf (S_COLOR_YELLOW"3DNow! ");
			if(CPUInfo[3] & 1<<30)
				Com_Printf (S_COLOR_YELLOW"ex3DNow! ");
			if(CPUInfo[3] & 1<<22)
				Com_Printf (S_COLOR_YELLOW"MmxExt ");
				
			if(MMX)
				Com_Printf (S_COLOR_YELLOW"MMX ");
			if(SSE)
				Com_Printf (S_COLOR_YELLOW"SSE ");
			if(SSE2)
				Com_Printf (S_COLOR_YELLOW"SSE2 ");
			if(SSE3)
				Com_Printf (S_COLOR_YELLOW"SSE3 ");
			if(SSE4)
				Com_Printf (S_COLOR_YELLOW"SSE4 ");
			if(HTT)
				Com_Printf (S_COLOR_YELLOW"HTT ");
			if(EM64T)
				Com_Printf (S_COLOR_YELLOW"EM64T");
			Com_Printf("\n");

        }
    }	

}
#define WIDTH 7

void GLimp_GetMemorySize(){

	MEMORYSTATUSEX		ramcheck;
	ramcheck.dwLength = sizeof (ramcheck);
	GlobalMemoryStatusEx(&ramcheck);

	Con_Printf (PRINT_ALL, "\n");
	Com_Printf("Memory loaded:            "S_COLOR_GREEN"%*ld"S_COLOR_WHITE" PCT\n",  WIDTH, ramcheck.dwMemoryLoad);
	Com_Printf("Total physical RAM:       "S_COLOR_GREEN"%*I64d"S_COLOR_WHITE" MB\n", WIDTH, ramcheck.ullTotalPhys >> 20);
	Com_Printf("Available physical RAM:   "S_COLOR_GREEN"%*I64d"S_COLOR_WHITE" MB\n", WIDTH, ramcheck.ullAvailPhys >> 20);
	Com_Printf("Total PageFile:           "S_COLOR_GREEN"%*I64d"S_COLOR_WHITE" MB\n", WIDTH, ramcheck.ullTotalPageFile >> 20);
	Com_Printf("Available PageFile:       "S_COLOR_GREEN"%*I64d"S_COLOR_WHITE" MB\n", WIDTH, ramcheck.ullAvailPageFile >> 20);
	Com_Printf("Total Virtual Memory:     "S_COLOR_GREEN"%*I64d"S_COLOR_WHITE" MB\n", WIDTH, ramcheck.ullTotalVirtual >> 20);
	Com_Printf("Available Virtual Memory: "S_COLOR_GREEN"%*I64d"S_COLOR_WHITE" MB\n", WIDTH, ramcheck.ullAvailVirtual >> 20);
	Con_Printf (PRINT_ALL, "\n");
}


BOOL Is64BitWindows()
{

 BOOL f64 = FALSE;
 return IsWow64Process(GetCurrentProcess(), &f64) && f64;

}

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);


/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/

/*============================
Fucking Microsoft!!!!
http://www.codeproject.com/Articles/678606/Part-Overcoming-Windows-s-deprecation-of-GetVe?msg=5080848#xx5080848xx
=============================*/
BOOL GetOsVersion(RTL_OSVERSIONINFOEXW* pk_OsVer)
{
	typedef LONG(WINAPI* tRtlGetVersion)(RTL_OSVERSIONINFOEXW*);
	LONG Status;

	memset(pk_OsVer, 0, sizeof(RTL_OSVERSIONINFOEXW));
	pk_OsVer->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOEXW);

	HMODULE h_NtDll = GetModuleHandleW(L"ntdll.dll");
	tRtlGetVersion f_RtlGetVersion = (tRtlGetVersion)GetProcAddress(h_NtDll, "RtlGetVersion");

	if (!f_RtlGetVersion)
		return FALSE; // This will never happen (all processes load ntdll.dll)

	Status = f_RtlGetVersion(pk_OsVer);
	return Status == 0; // STATUS_SUCCESS;
}

qboolean GLimp_Init( void *hinstance, void *wndproc )
{
	char		string[64], S[64];
	int			len;
	RTL_OSVERSIONINFOEXW    rtl_OsVer;
	OSVERSIONINFOEX			winver;
	SYSTEM_INFO				cpuinf;
	DWORD					prType;
	PGPI					pGPI;

	winver.dwOSVersionInfoSize = sizeof(winver);
	
	ZeroMemory(&cpuinf, sizeof(SYSTEM_INFO));
	GetSystemInfo(&cpuinf);


	Con_Printf (PRINT_ALL, "\n");
	Com_Printf ("========"S_COLOR_YELLOW"System Information"S_COLOR_WHITE"========\n");
	Con_Printf (PRINT_ALL, "\n");
	
	//CPU info
	GLimp_CpuID();
	//Memory info
	GLimp_GetMemorySize();

	if (GetOsVersion(&rtl_OsVer))
	{
	pGPI = (PGPI)GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");

	pGPI(rtl_OsVer.dwMajorVersion, rtl_OsVer.dwMinorVersion, 0, 0, &prType);

			switch (prType)
			{
			case PRODUCT_ULTIMATE:
				sprintf(S, "Ultimate Edition");
				break;
			case PRODUCT_ULTIMATE_E:
				sprintf(S, "Ultimate E Edition");
				break;
			case PRODUCT_ULTIMATE_N:
				sprintf(S, "Ultimate N Edition");
				break;
			case PRODUCT_HOME_PREMIUM:
				sprintf(S, "Home Premium Edition");
				break;
			case PRODUCT_HOME_PREMIUM_E:
				sprintf(S, "Home Premium E Edition");
				break;
			case PRODUCT_HOME_PREMIUM_N:
				sprintf(S, "Home Premium N Edition");
				break;
			case PRODUCT_HOME_BASIC:
				sprintf(S, "Home Basic Edition");
				break;
			case PRODUCT_HOME_BASIC_E:
				sprintf(S, "Home Basic E Edition");
				break;
			case PRODUCT_HOME_BASIC_N:
				sprintf(S, "Home Basic N Edition");
				break;
			case PRODUCT_ENTERPRISE:
				sprintf(S, "Enterprise Edition");
				break;
			case PRODUCT_ENTERPRISE_E:
				sprintf(S, "Enterprise E Edition");
				break;
			case PRODUCT_ENTERPRISE_N:
				sprintf(S, "Enterprise N Edition");
				break;
			case PRODUCT_BUSINESS:
				sprintf(S, "Business Edition");
				break;
			case PRODUCT_BUSINESS_N:
				sprintf(S, "Business N Edition");
				break;

			// =======win 10========
			case PRODUCT_PROFESSIONAL:
				sprintf(S, "Professional Edition");
				break;
			case PRODUCT_PROFESSIONAL_N:
				sprintf(S, "Professional N Edition");
				break;

			case PRODUCT_CORE:
				sprintf(S, "Home Edition"); 
				break;
			case PRODUCT_CORE_N:
				sprintf(S, "Home N Edition");
				break;
			case PRODUCT_CORE_SINGLELANGUAGE:
				sprintf(S, "Home Single Language");
				break;
			case PRODUCT_CORE_COUNTRYSPECIFIC:
				sprintf(S, "Home China Language");
				break; 
			//====================
			case PRODUCT_STARTER:
				sprintf(S, "Starter Edition");
				break;
			case PRODUCT_STARTER_E:
				sprintf(S, "Starter E Edition");
				break;
			case PRODUCT_STARTER_N:
				sprintf(S, "Starter N Edition");
				break;
			case PRODUCT_CLUSTER_SERVER:
				sprintf(S, "Cluster Server Edition");
				break;
			case PRODUCT_DATACENTER_SERVER:
				sprintf(S, "Datacenter Edition");
				break;
			case PRODUCT_DATACENTER_SERVER_CORE:
				sprintf(S, "Datacenter Edition (core installation)");
				break;
			case PRODUCT_ENTERPRISE_SERVER:
				sprintf(S, "Enterprise Edition");
				break;
			case PRODUCT_ENTERPRISE_SERVER_CORE:
				sprintf(S, "Enterprise Edition (core installation)");
				break;
			case PRODUCT_ENTERPRISE_SERVER_IA64:
				sprintf(S, "Enterprise Edition for Itanium-based Systems");
				break;
			case PRODUCT_SMALLBUSINESS_SERVER:
				sprintf(S, "Small Business Server");
				break;
			case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				sprintf(S, "Small Business Server Premium Edition");
				break;
			case PRODUCT_STANDARD_SERVER:
				sprintf(S, "Standard Edition");
				break;
			case PRODUCT_STANDARD_SERVER_CORE:
				sprintf(S, "Standard Edition (core installation)");
				break;
			case PRODUCT_WEB_SERVER:
				sprintf(S, "Web Server Edition");
				break;
			default:
				sprintf(S, "Ultimate Edition");
				break;
			}
			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 0) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Vista "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}
				else {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Vista "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 1) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 7 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 R2 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}
				else {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 7 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 R2 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 2) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}
				else {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 R2 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}

			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 3) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8.1 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}
				else {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8.1 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 R2 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}

			if (rtl_OsVer.dwMajorVersion == 10 && rtl_OsVer.dwMinorVersion == 0) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 10 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2016"S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}
				else {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 10 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2016 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
		}

	/*	if (GetVersionEx(&winver)){

			if (winver.dwMajorVersion == 5 && winver.dwMinorVersion == 2)
			{
				if (winver.wSuiteMask == VER_SUITE_STORAGE_SERVER)
					Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Storage Server 2003");

				if (winver.wProductType == VER_NT_WORKSTATION)
					Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows XP Professional "S_COLOR_GREEN"x64 "S_COLOR_YELLOW"Edition %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", winver.szCSDVersion, winver.dwBuildNumber);
				else {
					if (Is64BitWindows())
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 2003 Server "S_COLOR_GREEN"x64 "S_COLOR_YELLOW"Edition %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", winver.szCSDVersion, winver.dwBuildNumber);
					else
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 2003 Server "S_COLOR_GREEN"x32 "S_COLOR_YELLOW"Edition %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", winver.szCSDVersion, winver.dwBuildNumber);
				}


				if (winver.dwPlatformId == VER_PLATFORM_WIN32_NT)
				{

					if (winver.dwMajorVersion == 5 && winver.dwMinorVersion == 0)
						VID_Error(ERR_FATAL, "Quake2xp requires Windows XP or greater");

					if (winver.dwMajorVersion == 5 && winver.dwMinorVersion == 1)
					{
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows XP ");
						if (winver.wSuiteMask & VER_SUITE_PERSONAL)
							Com_Printf(S_COLOR_YELLOW, "Home Edition ");
						else
							Com_Printf(S_COLOR_YELLOW"Professional ");
						Com_Printf(S_COLOR_GREEN"%s ", winver.szCSDVersion);
						Com_Printf(S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", winver.dwBuildNumber);
					}
				}
			}
	}*/
else
	{
		Com_Printf( S_COLOR_RED "GLimp_Init() - GetVersionEx failed\n" );
		return qfalse;
	}
	
	// get user name
	len = sizeof(string);
	Com_Printf("\nUserName: "S_COLOR_GREEN"%s\n", GetUserName(string, &len) ? string : "");
	Com_Printf ("\n");

	glw_state.hInstance = ( HINSTANCE ) hinstance;
	glw_state.wndproc = wndproc;
	return qtrue;
	

}

/*
==================
GLW_InitExtensions

==================
*/

void GLW_InitExtensions() {

	qwglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)qwglGetProcAddress("wglGetExtensionsStringARB");

	if (!qwglGetExtensionsStringARB)
	{
		Com_Printf(S_COLOR_RED  "WGL extension string not found!");
		VID_Error(ERR_FATAL, "WGL extension string not found!");
	}

	glw_state.wglExtsString = qwglGetExtensionsStringARB(glw_state.hDCFake);

	if (glw_state.wglExtsString == NULL)
		Com_Printf(S_COLOR_RED "WGL_EXTENSION not found!\n");

	Com_Printf("\n");

	Com_Printf("=============================\n");
	Com_Printf(S_COLOR_GREEN"Checking Basic WGL Extensions\n");
	Com_Printf("=============================\n\n");

	if (strstr(glw_state.wglExtsString, "WGL_ARB_pixel_format"))
	{
		Com_Printf("...using WGL_ARB_pixel_format\n");
		qwglGetPixelFormatAttribivARB	= (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)	qwglGetProcAddress	("wglGetPixelFormatAttribivARB");
		qwglGetPixelFormatAttribfvARB	= (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)	qwglGetProcAddress	("wglGetPixelFormatAttribfvARB");
		qwglChoosePixelFormatARB		= (PFNWGLCHOOSEPIXELFORMATARBPROC)		qwglGetProcAddress	("wglChoosePixelFormatARB");

	}
	else {
		Com_Printf(S_COLOR_RED"WARNING!!! WGL_ARB_pixel_format not found\nOpenGL subsystem not initiation\n");
		VID_Error(ERR_FATAL, "WGL_ARB_pixel_format not found!");
	}

	if (strstr(glw_state.wglExtsString, "WGL_EXT_swap_control")) {
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)qwglGetProcAddress("wglSwapIntervalEXT");
		Com_Printf("...using WGL_EXT_swap_control\n");
	}
	else
		Com_Printf(S_COLOR_RED"...WGL_EXT_swap_control not found\n");

	gl_state.wgl_swap_control_tear = qfalse;
	if (strstr(glw_state.wglExtsString, "WGL_EXT_swap_control_tear")) {
		Com_Printf("...using WGL_EXT_swap_control_tear\n");
		gl_state.wgl_swap_control_tear = qtrue;
	}
	else {
		Com_Printf(S_COLOR_RED"WGL_EXT_swap_control_tear not found\n");
	}

	if (strstr(glw_state.wglExtsString, "WGL_ARB_multisample"))
		if (r_multiSamples->value < 2)
			Com_Printf("" S_COLOR_YELLOW "...ignoring WGL_ARB_multisample\n");
		else
			Com_Printf("...using WGL_ARB_multisample\n");
		
	if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context")) {
		qwglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)qwglGetProcAddress("wglCreateContextAttribsARB");

		if (qwglCreateContextAttribsARB)
			Com_Printf("...using WGL_ARB_create_context\n");
			else{
				Com_Printf(S_COLOR_RED"WARNING!!! WGL_ARB_create_context not found\nOpenGL subsystem not initiation\n");
				VID_Error(ERR_FATAL, "WGL_ARB_create_context not found!");
			}
		}

		if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context_profile"))
			Com_Printf("...using WGL_ARB_create_context_profile\n");
}

/*
==================
GLW_ShutdownFakeOpenGL

==================
*/

static void GLW_ShutdownFakeOpenGL(void) {

	if (glw_state.hGLRCFake) {
		if (qwglMakeCurrent)
			qwglMakeCurrent(NULL, NULL);
		if (qwglDeleteContext)
			qwglDeleteContext(glw_state.hGLRCFake);

		glw_state.hGLRCFake = NULL;
	}

	if (glw_state.hDCFake) {
		ReleaseDC(glw_state.hWndFake, glw_state.hDCFake);
		glw_state.hDCFake = NULL;
	}

	if (glw_state.hWndFake) {
		DestroyWindow(glw_state.hWndFake);
		glw_state.hWndFake = NULL;

		UnregisterClass(WINDOW_CLASS_FAKE, glw_state.hInstance);
	}
}

/*
==================
GLW_InitFakeOpenGL

==================
*/
static qboolean GLW_InitFakeOpenGL(void) {
	WNDCLASSEX				wndClass;
	PIXELFORMATDESCRIPTOR	PFD;
	int						pixelFormat;

	// register the frame class
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = 0;
	wndClass.lpfnWndProc = FakeWndProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = glw_state.hInstance;
	wndClass.hIcon = 0;
	wndClass.hIconSm = 0;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)COLOR_GRAYTEXT;
	wndClass.lpszMenuName = 0;
	wndClass.lpszClassName = WINDOW_CLASS_FAKE;

	if (!RegisterClassEx(&wndClass))
		return qfalse;

	// create the fake window
	glw_state.hWndFake = CreateWindowEx(0, WINDOW_CLASS_FAKE, WINDOW_NAME, WINDOW_STYLE, 0, 0, 320, 240, NULL, NULL, glw_state.hInstance, NULL);
	if (!glw_state.hWndFake) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	glw_state.hDCFake = GetDC(glw_state.hWndFake);
	if (!glw_state.hDCFake) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	// choose a pixel format
	memset(&PFD, 0, sizeof(PIXELFORMATDESCRIPTOR));

	PFD.cColorBits = 32;
	PFD.cDepthBits = 24;
	PFD.cStencilBits = 8;
	PFD.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	PFD.iLayerType = PFD_MAIN_PLANE;
	PFD.iPixelType = PFD_TYPE_RGBA;
	PFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	PFD.nVersion = 1;

	pixelFormat = ChoosePixelFormat(glw_state.hDCFake, &PFD);

	if (!pixelFormat) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	// set the pixel format
	DescribePixelFormat(glw_state.hDCFake, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &PFD);

	if (!SetPixelFormat(glw_state.hDCFake, pixelFormat, &PFD)) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	// create the fake GL context and make it current
	glw_state.hGLRCFake = qwglCreateContext(glw_state.hDCFake);

	if (!glw_state.hGLRCFake) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	if (!qwglMakeCurrent(glw_state.hDCFake, glw_state.hGLRCFake)) {
		GLW_ShutdownFakeOpenGL();
		return qfalse;
	}

	return qtrue;
}

/*
==================
GLW_DescribePixelFormat

==================
*/
static void GLW_DescribePixelFormat(int pixelFormat, glwPixelFormatDescriptor_t *pfd) {
	int	attribs[11], values[11];

	attribs[0]	= WGL_ACCELERATION_ARB;
	attribs[1]	= WGL_DRAW_TO_WINDOW_ARB;
	attribs[2]	= WGL_SUPPORT_OPENGL_ARB;
	attribs[3]	= WGL_DOUBLE_BUFFER_ARB;
	attribs[4]	= WGL_PIXEL_TYPE_ARB;
	attribs[5]	= WGL_COLOR_BITS_ARB;
	attribs[6]	= WGL_ALPHA_BITS_ARB;
	attribs[7]	= WGL_DEPTH_BITS_ARB;
	attribs[8]	= WGL_STENCIL_BITS_ARB;
	attribs[9]	= WGL_SAMPLE_BUFFERS_ARB;
	attribs[10] = WGL_SAMPLES_ARB;

	if (!qwglGetPixelFormatAttribivARB(glw_state.hDC, pixelFormat, 0, 11, attribs, values)) {
		// if failed, then multisampling is not supported
		qwglGetPixelFormatAttribivARB(glw_state.hDC, pixelFormat, 0, 9, attribs, values);

		values[9] = GL_FALSE;
		values[10] = 0;
	}

	pfd->accelerated	= (values[0] == WGL_FULL_ACCELERATION_ARB);
	pfd->drawToWindow	= (values[1] == GL_TRUE);
	pfd->supportOpenGL	= (values[2] == GL_TRUE);
	pfd->doubleBuffer	= (values[3] == GL_TRUE);
	pfd->rgba			= (values[4] == WGL_TYPE_RGBA_ARB);
	pfd->colorBits		= values[5];
	pfd->alphaBits		= values[6];
	pfd->depthBits		= values[7];
	pfd->stencilBits	= values[8];
	pfd->samples		= (values[9] == GL_TRUE) ? values[10] : 0;
}

/*
==================
GLW_ChoosePixelFormat

==================
*/
static int GLW_ChoosePixelFormat(int colorBits, int alphaBits, int depthBits, int stencilBits, int samples) {
	glwPixelFormatDescriptor_t	current, selected;
	int							i, numPixelFormats, pixelFormat = 0;

	// initialize the fake OpenGL stuff so that we can use
	// the extended pixel format functionality
	if (!GLW_InitFakeOpenGL()) {
		Com_Printf(S_COLOR_RED "...failed to initialize fake OpenGL context\n");
		return 0;
	}

	// check the necessary extensions
	GLW_InitExtensions();
	
	Com_Printf(S_COLOR_YELLOW"\n...Attempting PIXELFORMAT:\n\n");
	
	// count pixel formats
	numPixelFormats = WGL_NUMBER_PIXEL_FORMATS_ARB;
	qwglGetPixelFormatAttribivARB(glw_state.hDC, 0, 0, 1, &numPixelFormats, &numPixelFormats);

	if (numPixelFormats < 1) {
		Com_Printf(S_COLOR_RED "...no pixel formats found\n");
		GLW_ShutdownFakeOpenGL();
		return 0;
	}

	Com_Printf("..." S_COLOR_GREEN "%i " S_COLOR_WHITE "pixel formats found\n", numPixelFormats);
	
	// check if multisampling is desired
	if (samples > 0) 
		Com_Printf("...attempting to use multisampling\n");
	else
		samples = 0;

	// run through all the pixel formats, looking for the best match
	for (i = 1; i <= numPixelFormats; i++) {
		// describe the current pixel format
		GLW_DescribePixelFormat(i, &current);

		// check acceleration
		if (!current.accelerated) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, software emulation\n", i);
			continue;
		}

		// check samples
		if (current.samples && !samples) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, multisample\n", i);
			continue;
		}

		// check flags
		if (!current.drawToWindow || !current.supportOpenGL || !current.doubleBuffer) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, improper flags\n", i);
			continue;
		}

		// check pixel type
		if (!current.rgba) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, not RGBA\n", i);
			continue;
		}

		// check color bits
		if (current.colorBits < colorBits) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, insufficient color bits (" S_COLOR_RED "%i " S_COLOR_WHITE "< " S_COLOR_RED "%i" S_COLOR_WHITE ")\n", i, current.colorBits, colorBits);
			continue;
		}

		// check alpha bits
		if (current.alphaBits < alphaBits) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, insufficient alpha bits (" S_COLOR_RED "%i " S_COLOR_WHITE "< " S_COLOR_RED "%i" S_COLOR_WHITE ")\n", i, current.alphaBits, alphaBits);
			continue;
		}

		// check depth bits
		if (current.depthBits < depthBits) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, insufficient depth bits (" S_COLOR_RED "%i " S_COLOR_WHITE "< " S_COLOR_RED "%i" S_COLOR_WHITE ")\n", i, current.depthBits, depthBits);
			continue;
		}

		// check stencil bits
		if (current.stencilBits < stencilBits) {
			Com_DPrintf("...PIXELFORMAT " S_COLOR_RED "%i " S_COLOR_WHITE "rejected, insufficient stencil bits (" S_COLOR_RED "%i " S_COLOR_WHITE "< " S_COLOR_RED "%i" S_COLOR_WHITE ")\n", i, current.stencilBits, stencilBits);
			continue;
		}

		// if we don't have a selected pixel format yet, then use it
		if (!pixelFormat) {
			selected = current;
			pixelFormat = i;
			continue;
		}

		// if current pixel format is better than selected pixel format, then use it
		if (selected.samples != samples) {
			if (current.samples == samples || current.samples > selected.samples) {
				selected = current;
				pixelFormat = i;
				continue;
			}
		}

		if (selected.colorBits != colorBits) {
			if (current.colorBits == colorBits || current.colorBits > selected.colorBits) {
				selected = current;
				pixelFormat = i;
				continue;
			}
		}

		if (selected.alphaBits != alphaBits) {
			if (current.alphaBits == alphaBits || current.alphaBits > selected.alphaBits) {
				selected = current;
				pixelFormat = i;
				continue;
			}
		}

		if (selected.depthBits != depthBits) {
			if (current.depthBits == depthBits || current.depthBits > selected.depthBits) {
				selected = current;
				pixelFormat = i;
				continue;
			}
		}

		if (selected.stencilBits != stencilBits) {
			if (current.stencilBits == stencilBits || current.stencilBits > selected.stencilBits) {
				selected = current;
				pixelFormat = i;
				continue;
			}
		}
	}

	// shutdown the fake OpenGL stuff since we no longer need it
	GLW_ShutdownFakeOpenGL();

	// make sure we have a valid pixel format
	if (!pixelFormat) {
		Com_Printf(S_COLOR_RED "...no hardware acceleration found\n");
		return 0;
	}

	Com_Printf("...hardware acceleration found\n");

	// report if multisampling is desired but unavailable
	if (samples && !selected.samples)
		Com_Printf(S_COLOR_MAGENTA"...failed to find a PIXELFORMAT with multisampling\n");

	gl_config.colorBits		= selected.colorBits;
	gl_config.alphaBits		= selected.alphaBits;
	gl_config.depthBits		= selected.depthBits;
	gl_config.stencilBits	= selected.stencilBits;
	gl_config.samples		= selected.samples;
	
	Com_Printf("...Selected " S_COLOR_GREEN "%i " S_COLOR_WHITE "PIXELFORMAT\n", pixelFormat);

	return pixelFormat;
}

qboolean GLW_InitDriver(void) {
	PIXELFORMATDESCRIPTOR	PFD;
	
	int	pixelFormat;
	int	debugFlag	= r_glDebugOutput->value ? WGL_CONTEXT_DEBUG_BIT_ARB : 0;
	int	contextMask = r_glCoreProfile->value ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
	int	attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, r_glMajorVersion->value,
		WGL_CONTEXT_MINOR_VERSION_ARB, r_glMinorVersion->value,
		WGL_CONTEXT_FLAGS_ARB,  debugFlag,
		WGL_CONTEXT_PROFILE_MASK_ARB, contextMask,
		0
	};
	const char *profileName[] = { "core", "compatibility" };
	
	// get a DC for the current window
	Com_Printf("...getting DC: ");

	glw_state.hDC = GetDC(glw_state.hWnd);
	if (!glw_state.hDC) {
		Com_Printf(S_COLOR_RED "failed\n");
		return qfalse;
	}

	Com_Printf(S_COLOR_GREEN"ok\n");
	
	int samples;
	if ((int)r_multiSamples->value == 1)
		samples = 0;
	else
		samples = (int)r_multiSamples->value;

	// choose a pixel format
	pixelFormat = GLW_ChoosePixelFormat(32, 8, 24, 8, samples);
	
	if (!pixelFormat) {
		Com_Printf(S_COLOR_RED "...failed to find an appropriate PIXELFORMAT\n");

		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;

		return qfalse;
	}

	// describe the pixel format
	Com_Printf("...setting pixel format: ");

	qwglDescribePixelFormat(glw_state.hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &PFD);

	// set the pixel format
	if (!qwglSetPixelFormat(glw_state.hDC, pixelFormat, &PFD)) {
		Com_Printf(S_COLOR_RED "failed\n");

		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;

		return qfalse;
	}

	Com_Printf("ok\n");

	// create the GL context
	Com_Printf("...creating openGL " S_COLOR_GREEN "%i.%i" S_COLOR_YELLOW "  %s" S_COLOR_WHITE " profile context: ", (int)r_glMajorVersion->value, (int)r_glMinorVersion->value, profileName[contextMask == WGL_CONTEXT_CORE_PROFILE_BIT_ARB ? 0 : 1]);

	glw_state.hGLRC = qwglCreateContextAttribsARB(glw_state.hDC, NULL, attribs);

	if (!glw_state.hGLRC) {
		if (GetLastError() == ERROR_INVALID_VERSION_ARB)
			Com_Error(ERR_FATAL, "Current video card/driver combination does not support OpenGL %i.%i", r_glMajorVersion->value, r_glMinorVersion->value);

		Com_Printf(S_COLOR_RED "failed\n");

		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;

		return qfalse;
	}

	Com_Printf(S_COLOR_GREEN "ok\n");

	// make it current
	Com_Printf("...making context current: ");

	if (!qwglMakeCurrent(glw_state.hDC, glw_state.hGLRC)) {
		Com_Printf(S_COLOR_RED "failed\n");

		qwglDeleteContext(glw_state.hGLRC);
		glw_state.hGLRC = NULL;

		ReleaseDC(glw_state.hWnd, glw_state.hDC);
		glw_state.hDC = NULL;

		return qfalse;
	}

	Com_Printf(S_COLOR_GREEN "ok\n");

	qglGetIntegerv(GL_MAX_SAMPLES, &gl_config.maxSamples);

	Com_Printf("\nPIXELFORMAT: Color "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits, Depth "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits, Alpha "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits,\n             Stencil "S_COLOR_GREEN"%i"S_COLOR_WHITE"-bits, MSAA [" S_COLOR_GREEN "%i" S_COLOR_WHITE " max] [" S_COLOR_GREEN "%i"S_COLOR_WHITE" selected]\n",
				gl_config.colorBits, gl_config.depthBits, gl_config.alphaBits, gl_config.stencilBits, gl_config.maxSamples, gl_config.samples);

	gl_config.glMajorVersion = r_glMajorVersion->value;
	gl_config.glMinorVersion = r_glMinorVersion->value;

	return qtrue;
}


/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/


void GLimp_EndFrame (void)
{
	if ( !qwglSwapBuffers( glw_state.hDC ) )
			VID_Error( ERR_FATAL, "GLimp_EndFrame() - SwapBuffers() failed!\n" );

	r_newrefdef.time=Sys_Milliseconds() * 0.001f;
	ref_realtime=Sys_Milliseconds() * 0.0005f;
}


void GL_UpdateSwapInterval()
{

	if(r_vsync->modified)
	r_vsync->modified = qfalse;

	if(gl_state.wgl_swap_control_tear){
	
	if (wglSwapIntervalEXT){
		if(r_vsync->value >=2)
			wglSwapIntervalEXT(-1);
	else if(r_vsync->value >=1)
			wglSwapIntervalEXT(1);	
	else
			wglSwapIntervalEXT(0);
		}
	}
	else
		if (wglSwapIntervalEXT)
			wglSwapIntervalEXT(r_vsync->value);
	
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( qboolean active )
{
	if ( active )
	{
		SetForegroundWindow( glw_state.hWnd );
		ShowWindow( glw_state.hWnd, SW_RESTORE );
	}
	else
	{
		if ( r_fullScreen->value )
			ShowWindow( glw_state.hWnd, SW_MINIMIZE );
	}
}
