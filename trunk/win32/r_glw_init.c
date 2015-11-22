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

#include <assert.h>
#include <windows.h>
#include "../ref_gl/r_local.h"
#include "glw_win.h"
#include "winquake.h"
#include <intrin.h>


static qboolean GLimp_SwitchFullscreen( int width, int height );
qboolean GLimp_InitGL (void);

glwstate_t glw_state;

/*
** VID_CreateWindow
*/
#define	WINDOW_CLASS_NAME	"Quake2xp"
#define	OPENGL_CLASS		"OpenGLDummyPFDWindow"
qboolean VID_CreateWindow( int width, int height, qboolean fullscreen )
{
	WNDCLASS		wc;
	RECT			r;
	cvar_t			*vid_xpos, *vid_ypos;
	int				stylebits;
	int				x, y, w, h;
	int				exstyle;

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

	r.left = 0;
	r.top = 0;
	r.right  = width;
	r.bottom = height;

	AdjustWindowRect (&r, stylebits, FALSE);

	w = r.right - r.left;
	h = r.bottom - r.top;

	if (fullscreen)
	{
		x = 0;
		y = 0;
	}
	else
	{
		vid_xpos = Cvar_Get ("vid_xpos", "0", 0);
		vid_ypos = Cvar_Get ("vid_ypos", "0", 0);
		x = vid_xpos->value;
		y = vid_ypos->value;
	}

	glw_state.hWnd = CreateWindowEx (
		 exstyle, 
		 WINDOW_CLASS_NAME,
		 "quake2xp",
		 stylebits,
		 x, y, w, h,
		 NULL,
		 NULL,
		 glw_state.hInstance,
		 NULL);

	if (!glw_state.hWnd)
		VID_Error (ERR_FATAL, "Couldn't create window");

	ShowWindow( glw_state.hWnd, SW_SHOW );
	UpdateWindow( glw_state.hWnd );

	// init all the gl stuff for the window
	if (!GLimp_InitGL ())
	{
		Com_Printf(S_COLOR_RED "VID_CreateWindow() - GLimp_InitGL failed\n");
		return qfalse;
	}

	SetForegroundWindow( glw_state.hWnd );
	SetFocus( glw_state.hWnd );

	// let the sound and input subsystems know about the new window
	VID_NewWindow (width, height);

	return qtrue;
}


/*
** GLimp_SetMode
*/

rserr_t GLimp_SetMode( unsigned *pwidth, unsigned *pheight, int mode, qboolean fullscreen )
{
	int width, height;
	const char *win_fs[] = { "Window", "Full Screen" };
	// get current device
	HDC hdc = GetDC (NULL);

	 //read current display mode
    glw_state.desktopWidth =  GetDeviceCaps(hdc, HORZRES);
    glw_state.desktopHeight = GetDeviceCaps(hdc, VERTRES);

	Com_Printf("Initializing OpenGL display\n");
	
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
		DEVMODE dm;

		Com_Printf("...attempting fullscreen\n" );

		memset( &dm, 0, sizeof( dm ) );

		dm.dmSize = sizeof( dm );

		dm.dmPelsWidth  = width;
		dm.dmPelsHeight = height;
		dm.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;
        
 
		/* display frequency */

		if (r_displayRefresh->value !=0 )
		{
	        gl_state.displayrefresh	= r_displayRefresh->value;
			dm.dmDisplayFrequency	= r_displayRefresh->value;
			dm.dmFields				|= DM_DISPLAYFREQUENCY;
			Com_Printf("...display frequency is "S_COLOR_GREEN"%d"S_COLOR_WHITE" hz\n", gl_state.displayrefresh);
		}
		else 
		
		{
			int displayref = GetDeviceCaps (hdc, VREFRESH);
            dm.dmDisplayFrequency	= displayref;
			dm.dmFields				|= DM_DISPLAYFREQUENCY;
			Com_Printf("...using desktop frequency: "S_COLOR_GREEN"%d"S_COLOR_WHITE" hz\n", displayref);
		}
     
			// force set 32-bit color depth
			dm.dmBitsPerPel = 32;
			dm.dmFields |= DM_BITSPERPEL;
	
			
		Con_Printf( PRINT_ALL, "...calling CDS: " );
		if ( ChangeDisplaySettings( &dm, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL )
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
			if ( ChangeDisplaySettings( &dm, CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
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
       		
		Com_Printf ("Cpu Brand Name: "S_COLOR_GREEN"%s\n", &CPUBrandString[8]);
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

	//set high process priority for fullscreen mode
	if(r_fullScreen->value && sys_priority->value && cpuinf.dwNumberOfProcessors > 1)
		SetPriorityClass (GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	else
		SetPriorityClass (GetCurrentProcess(), NORMAL_PRIORITY_CLASS);


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
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

				if (Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Vista "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 1) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 7 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 R2 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

				if (Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 7 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2008 R2 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 2) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

				if (Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 R2 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}

			if (rtl_OsVer.dwMajorVersion == 6 && rtl_OsVer.dwMinorVersion == 3) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8.1 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

				if (Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 8.1 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2012 R2 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
		if (GetOsVersion(&rtl_OsVer))
			if (rtl_OsVer.dwMajorVersion == 10 && rtl_OsVer.dwMinorVersion == 0) {
				if (!Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 10 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2016"S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

				if (Is64BitWindows()) {

					if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 10 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
					if (rtl_OsVer.wProductType != VER_NT_WORKSTATION)
						Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2016 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, rtl_OsVer.szCSDVersion, rtl_OsVer.dwBuildNumber);
				}

			}
		}

		if (GetVersionEx(&winver)){

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
	}
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
==============================
MULTISAMPLING  SUPPORT
BASED ON  Richard Stanway CODE
THX, RICHARD!!!
==============================
*/

// don't do anything
static LRESULT CALLBACK StupidOpenGLProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
  return DefWindowProc(hWnd,msg,wParam,lParam);
}

// Registers the window classes
qboolean RegisterOpenGLWindow(HINSTANCE hInst)
{
  WNDCLASSEX wcex;

  // Initialize our Window class
  wcex.cbSize = sizeof(wcex);
  if (GetClassInfoEx(hInst, OPENGL_CLASS, &wcex))
	return TRUE;

  // register main one
  ZeroMemory(&wcex,sizeof(wcex));

  // now the stupid one
  wcex.cbSize			= sizeof(wcex);
  wcex.style			= CS_OWNDC;
  wcex.cbWndExtra		= 0; // space for our pointer 
  wcex.lpfnWndProc		= StupidOpenGLProc;
  wcex.hbrBackground	= NULL;
  wcex.hInstance		= hInst;
  wcex.hCursor			= LoadCursor(NULL,IDC_ARROW);
  wcex.lpszClassName	= OPENGL_CLASS;

  return RegisterClassEx(&wcex);
}

qboolean GLimp_InitGL (void)
{
		int			iAttributes[30];
		float		fAttributes[] = {0, 0};
		int			iResults[30];
		int			pixelFormat;
		uint		numFormats;
		int			status;
		int			major, minor;
		int			attribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB, 
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB/*WGL_CONTEXT_CORE_PROFILE_BIT_ARB*/,
			0
		};

		PIXELFORMATDESCRIPTOR temppfd = { 
			sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
			1,								// version number
			PFD_DRAW_TO_WINDOW |			// support window
			PFD_SUPPORT_OPENGL |			// support OpenGL
			PFD_DOUBLEBUFFER,				// double buffered
			PFD_TYPE_RGBA,					// RGBA type
			32,								// 32-bit color depth	FOREVER :)
			0, 0, 0, 0, 0, 0,				// color bits ignored
			0,								// no alpha buffer
			0,								// shift bit ignored
			0,								// no accumulation buffer
			0, 0, 0, 0, 					// accum bits ignored
			24,								// 24-bit z-buffer		FOREVER :)
			0,								/// Berserker: нам уже не нужен stencil		// 8-bit stencil buffer	FOREVER :)
			0,								// no auxiliary buffer
			PFD_MAIN_PLANE,					// main layer
			0,								// reserved
			0, 0, 0							// layer masks ignored
		};

		RegisterOpenGLWindow(glw_state.hInstance);

		HGLRC hGLRC;
		HWND temphwnd = CreateWindowEx(0L,OPENGL_CLASS,"Quake2xp OpenGL PFD Detection Window",WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,0,0,1,1,glw_state.hWnd,0,glw_state.hInstance,NULL);
		HDC hDC = GetDC (temphwnd);

		// Set up OpenGL
		pixelFormat = ChoosePixelFormat(hDC, &temppfd);				
		
		if (!pixelFormat)
		{
			Com_Printf (S_COLOR_RED "GLimp_Init() - ChoosePixelFormat (%d - color /%d- depth /%d - alpha /%d - stencil) failed. Error %d.\n", (int)temppfd.cColorBits, (int)temppfd.cColorBits, (int)temppfd.cAlphaBits, (int)temppfd.cStencilBits, GetLastError());
			VID_Error (ERR_FATAL,  "GLimp_Init() - ChoosePixelFormat ((%d - color /%d- depth /%d - alpha /%d - stencil) failed. Error %d.\n", (int)temppfd.cColorBits, (int)temppfd.cColorBits, (int)temppfd.cAlphaBits, (int)temppfd.cStencilBits, GetLastError());
			
		}

		if (SetPixelFormat(hDC, pixelFormat, &temppfd) == FALSE)
		{
			Com_Printf (S_COLOR_RED "GLimp_Init() - SetPixelFormat (%d) failed. Error %d.\n", pixelFormat, GetLastError());
			VID_Error (ERR_FATAL,  "GLimp_Init() - SetPixelFormat (%d) failed. Error %d.\n", pixelFormat, GetLastError());
		}

		// Create a rendering context
		hGLRC = qwglCreateContext(hDC);
		if (!hGLRC) {
			Com_Printf (S_COLOR_RED "GLimp_Init() - qwglCreateContext failed\n");
			VID_Error (ERR_FATAL,  "GLimp_Init() - qwglCreateContext failed\n");
		}

		// Make the rendering context current
		if (!(qwglMakeCurrent(hDC, hGLRC))) {
			
			Com_Printf (S_COLOR_RED	"GLimp_Init() - qwglMakeCurrent failed\n");
			VID_Error (ERR_FATAL,	"GLimp_Init() - qwglMakeCurrent failed\n");
		}
		
		qwglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)qwglGetProcAddress("wglGetExtensionsStringARB");
		
		if (!qwglGetExtensionsStringARB) 
			{
			Com_Printf (S_COLOR_RED  "WGL extension string not found!");
			VID_Error (ERR_FATAL, "WGL extension string not found!");
			}
		
		glw_state.wglExtsString = qwglGetExtensionsStringARB (hDC);
		
		if (glw_state.wglExtsString == NULL)
			Com_Printf (S_COLOR_RED "WGL_EXTENSION not found!\n");
		
		Com_Printf("\n");

	Com_Printf ("=============================\n");
	Com_Printf (S_COLOR_GREEN"Checking Basic WGL Extensions\n");
	Com_Printf ("=============================\n\n");

	if ( strstr( glw_state.wglExtsString, "WGL_ARB_pixel_format" ) )
	{
		Com_Printf("...using WGL_ARB_pixel_format\n");
		qwglGetPixelFormatAttribivARB	= (PFNWGLGETPIXELFORMATATTRIBIVARBPROC)	qwglGetProcAddress("wglGetPixelFormatAttribivARB");
        qwglGetPixelFormatAttribfvARB	= (PFNWGLGETPIXELFORMATATTRIBFVARBPROC)	qwglGetProcAddress("wglGetPixelFormatAttribfvARB");
        qwglChoosePixelFormatARB		= (PFNWGLCHOOSEPIXELFORMATARBPROC)		qwglGetProcAddress("wglChoosePixelFormatARB");
	
	} 
	else {
		Com_Printf(S_COLOR_RED"WARNING!!! WGL_ARB_pixel_format not found\nOpenGL subsystem not initiation\n");
		VID_Error (ERR_FATAL, "WGL_ARB_pixel_format not found!");
		}

	if (strstr(glw_state.wglExtsString, "WGL_EXT_swap_control")) {
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) qwglGetProcAddress("wglSwapIntervalEXT");
		Com_Printf("...using WGL_EXT_swap_control\n");
	} else 
		Com_Printf(S_COLOR_RED"...WGL_EXT_swap_control not found\n");

	gl_state.wgl_swap_control_tear = qfalse;
	if ( strstr( glw_state.wglExtsString, "WGL_EXT_swap_control_tear")){
		Com_Printf("...using WGL_EXT_swap_control_tear\n");
		gl_state.wgl_swap_control_tear = qtrue;
	} 
	else {
		Com_Printf(S_COLOR_RED"WGL_EXT_swap_control_tear not found\n");
	}

	gl_state.arb_multisample = qfalse;
	if ( strstr(  glw_state.wglExtsString, "WGL_ARB_multisample" ) )
	if(r_arbSamples->value < 2)
		{
			Com_Printf(""S_COLOR_YELLOW"...ignoring WGL_ARB_multisample\n");
			gl_state.arb_multisample = qfalse;
		}else
	{
			Com_Printf("...using WGL_ARB_multisample\n");
			gl_state.arb_multisample = qtrue;
		
	}
	else
	{
		Com_Printf("...WGL_ARB_multisample not found\n");
		gl_state.arb_multisample = qfalse;
	}

	if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context")){
		qwglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)qwglGetProcAddress("wglCreateContextAttribsARB");
		
		if (qwglCreateContextAttribsARB)
			Com_Printf("...using WGL_ARB_create_context\n");
		else
		{
			Com_Printf(S_COLOR_RED"WARNING!!! WGL_ARB_create_context not found\nOpenGL subsystem not initiation\n");
			VID_Error(ERR_FATAL, "WGL_ARB_create_context not found!");
		}

	}
	
	if (strstr(glw_state.wglExtsString, "WGL_ARB_create_context_profile"))
		Com_Printf("...using WGL_ARB_create_context_profile\n");
	

	qglGetIntegerv(GL_MAJOR_VERSION, &major);
	qglGetIntegerv(GL_MINOR_VERSION, &minor);

	if (major < 3 && minor < 3){
		Com_Printf(S_COLOR_RED"Quake2xp requires OpenGL version 3.3 or higher.\nProbably your graphics card is unsupported or the drivers are not up-to-date.\nCurrent GL version is %i.%i\n", major, minor);
		VID_Error(ERR_FATAL, "Quake2xp requires OpenGL version 3.3 or higher.\nProbably your graphics card is unsupported or the drivers are not up-to-date.\nCurrent GL version is %i.%i\n", major, minor);
	}
	glGetStringi = (PFNGLGETSTRINGIPROC)qwglGetProcAddress("glGetStringi");
	if (!glGetStringi)
		Com_Printf(S_COLOR_RED"glGetStringi: bad getprocaddress\n");

	// Make no rendering context current and destroy the rendering context...
	qwglMakeCurrent(NULL, NULL);
	qwglDeleteContext(hGLRC);
	hGLRC = NULL;

	Com_Printf("\n=============================\n\n");

	// Get the number of pixel format available
		iAttributes[0] = WGL_NUMBER_PIXEL_FORMATS_ARB;

		if (qwglGetPixelFormatAttribivARB(hDC, 0, 0, 1, iAttributes, iResults) == GL_FALSE) {
			
			Com_Printf (S_COLOR_RED "GLimp_InitGL() - wglGetPixelFormatAttribivARB failed\n");
			VID_Error (ERR_FATAL,  "GLimp_InitGL() - wglGetPixelFormatAttribivARB failed\n");
			
		}
		
		// Choose a Pixel Format Descriptor (PFD) with multisampling support.
		
		iAttributes[0] = WGL_DOUBLE_BUFFER_ARB;
		iAttributes[1] = TRUE;
		
		iAttributes[2] = WGL_COLOR_BITS_ARB;
		iAttributes[3] = 32;

		iAttributes[4] = WGL_DEPTH_BITS_ARB;
		iAttributes[5] = 24;

		iAttributes[6] = WGL_ALPHA_BITS_ARB;
		iAttributes[7] = 8;

		iAttributes[8] = WGL_STENCIL_BITS_ARB;
		iAttributes[9] = 8;
		
		iAttributes[10] = gl_state.arb_multisample ? WGL_SAMPLE_BUFFERS_ARB : 0;
		iAttributes[11] = gl_state.arb_multisample ? TRUE : 0;
		
		iAttributes[12] = gl_state.arb_multisample ? WGL_SAMPLES_ARB : 0;
		iAttributes[13] = gl_state.arb_multisample ? (int)r_arbSamples->value : 0;

//		iAttributes[14] = WGL_SWAP_EXCHANGE_ARB;
//		iAttributes[15] = TRUE;

		// First attempt...
		status = qwglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &pixelFormat, &numFormats);
		// Failure happens not only when the function fails, but also when no matching pixel format has been found
		if (status == FALSE || numFormats == 0)
		{
			
			Com_Printf (S_COLOR_RED "GLimp_InitGL() - wglChoosePixelFormatARB failed\n");
			VID_Error (ERR_FATAL,  "GLimp_InitGL() - wglChoosePixelFormatARB failed\n");
	
		}
		else
		{
			// Fill the list of attributes we are interested in
			iAttributes[0 ] = WGL_PIXEL_TYPE_ARB;
			iAttributes[1 ] = WGL_COLOR_BITS_ARB;
			iAttributes[2 ] = WGL_RED_BITS_ARB;
			iAttributes[3 ] = WGL_GREEN_BITS_ARB;
			iAttributes[4 ] = WGL_BLUE_BITS_ARB;
			iAttributes[5 ] = WGL_ALPHA_BITS_ARB;
			iAttributes[6 ] = WGL_DEPTH_BITS_ARB;
			iAttributes[7 ] = WGL_STENCIL_BITS_ARB;

			// Since WGL_ARB_multisample and WGL_pbuffer are extensions, we must check if
			// those extensions are supported before passing the corresponding enums
			// to the driver. This could cause an error if they are not supported.
			iAttributes[8 ] = gl_state.arb_multisample ? WGL_SAMPLE_BUFFERS_ARB : WGL_PIXEL_TYPE_ARB;
			iAttributes[9 ] = gl_state.arb_multisample ? WGL_SAMPLES_ARB : WGL_PIXEL_TYPE_ARB;
			
			iAttributes[10] = WGL_DRAW_TO_WINDOW_ARB;
			iAttributes[11] = WGL_DRAW_TO_BITMAP_ARB;
			iAttributes[12] = WGL_PIXEL_TYPE_ARB;
			iAttributes[13] = WGL_DOUBLE_BUFFER_ARB;
			iAttributes[14] = WGL_STEREO_ARB;
			iAttributes[15] = WGL_ACCELERATION_ARB;
			iAttributes[16] = WGL_NEED_PALETTE_ARB;
			iAttributes[17] = WGL_NEED_SYSTEM_PALETTE_ARB;
			iAttributes[18] = WGL_SWAP_LAYER_BUFFERS_ARB;
			iAttributes[19] = WGL_SWAP_METHOD_ARB;
			iAttributes[20] = WGL_NUMBER_OVERLAYS_ARB;
			iAttributes[21] = WGL_NUMBER_UNDERLAYS_ARB;
			iAttributes[22] = WGL_TRANSPARENT_ARB;
			iAttributes[23] = WGL_SUPPORT_GDI_ARB;
			iAttributes[24] = WGL_SUPPORT_OPENGL_ARB;

			if (qwglGetPixelFormatAttribivARB(hDC, pixelFormat, 0, 25, iAttributes, iResults) == GL_FALSE) {
				Com_Printf (S_COLOR_RED	"GLimp_InitGL() wglGetPixelFormatAttribivARB failed\n");
				VID_Error (ERR_FATAL,	"GLimp_InitGL() wglGetPixelFormatAttribivARB failed\n"); 
				
			}

			Com_Printf ("WGL_PFD: Color "S_COLOR_GREEN"%d"S_COLOR_WHITE"-bits, Depth "S_COLOR_GREEN"%d"S_COLOR_WHITE"-bits, Alpha "S_COLOR_GREEN"%d"S_COLOR_WHITE"-bits, Stencil "S_COLOR_GREEN"%d"S_COLOR_WHITE"-bits \n",
				iResults[1], iResults[6], iResults[5], iResults[7]);

			if (iResults[8])
				if (gl_state.arb_multisample && r_arbSamples->value >1)
							Com_Printf ( "using multisampling, "S_COLOR_GREEN"%d"S_COLOR_WHITE" samples per pixel\n", iResults[9]);

			if (iResults[15] != WGL_FULL_ACCELERATION_ARB)
				Com_Printf ( S_COLOR_RED "********** WARNING **********\npixelformat %d is NOT hardware accelerated!\n*****************************\n", pixelFormat);
			
			ReleaseDC (temphwnd, hDC);
			DestroyWindow (temphwnd);
			temphwnd = NULL;


			if ( ( glw_state.hDC = GetDC( glw_state.hWnd ) ) == NULL )
			{
				Com_Printf(S_COLOR_RED "GLimp_InitGL() GetDC failed\n" );
				return qfalse;
			}

			SetPixelFormat (glw_state.hDC, pixelFormat, &temppfd);

			if ((glw_state.hGLRC = /*qwglCreateContext( glw_state.hDC )*/qwglCreateContextAttribsARB(glw_state.hDC, 0, attribs)) == 0) // create compatibility gl 3.3 context
			{
				Com_Printf (S_COLOR_RED "GLimp_InitGL() qwglCreateContextAttribsARB failed (%d)\n", GetLastError());
				goto fail;
			}

			if ( !qwglMakeCurrent( glw_state.hDC, glw_state.hGLRC ) )
			{
				Com_Printf (S_COLOR_RED "GLimp_InitGL() qwglMakeCurrent failed\n");
				goto fail;
			}

		
		}

return qtrue;

fail:
	if ( glw_state.hGLRC )
	{
		qwglDeleteContext( glw_state.hGLRC );
		glw_state.hGLRC = NULL;
	}

	if ( glw_state.hDC )
	{
		ReleaseDC( glw_state.hWnd, glw_state.hDC );
		glw_state.hDC = NULL;
	}
	return qfalse;
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
