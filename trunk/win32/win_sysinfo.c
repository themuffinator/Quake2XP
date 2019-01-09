/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

#include "../ref_gl/r_local.h"
#include "win_languages.h"

#define UI_NUM_LANGS ( sizeof( ui_Language ) / sizeof( ui_Language[0] ) )

void Sys_WindowsInfo() {
	
	LANGID	lang = GetUserDefaultUILanguage();
	int		len, len2;
	char	s[64], s2[MAX_COMPUTERNAME_LENGTH + 1];

	len = sizeof(s);
	len2 = sizeof(s2);
	
	for (int i = 0; i < UI_NUM_LANGS; i++) {
		if (lang == ui_Language[i].num) {
			Com_Printf("\nUI Language:   "S_COLOR_YELLOW"%s\n", ui_Language[i].description);
			break;
		}
	}

	Com_Printf("User Name:     "S_COLOR_YELLOW"%s\n", GetUserName(s, &len) ? s : "");
	Com_Printf("Computer Name: "S_COLOR_YELLOW"%s\n", GetComputerName(s2, &len2) ? s2 : "");
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
		mov dword ptr[start + 0], eax
		mov dword ptr[start + 4], edx
	}

	QueryPerformanceCounter((LARGE_INTEGER *)&stop);
	stop += frequency;

	do {
		QueryPerformanceCounter((LARGE_INTEGER *)&counter);
	} while (counter < stop);

	__asm {
		rdtsc
		mov dword ptr[end + 0], eax
		mov dword ptr[end + 4], edx
	}

	speed = (unsigned)((end - start) / 1000000);
	return speed;

}


qboolean GetCpuCoresThreads(qboolean ryzen) {
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
	hyperThreads = cpuFeatures & BIT(28) && cores < logical;

	if (ryzen)
	{
		Com_Printf("Cores/Threads: "S_COLOR_GREEN"%i"S_COLOR_WHITE"|"S_COLOR_GREEN"%i\n", cores >> 1, logical);
		return qtrue;
	}
	else
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

void Sys_CpuID()
{
	char		CPUString[0x20];
	char		CPUBrandString[0x40];
	int			CPUInfo[4] = { -1 };
	int			nFeatureInfo = 0;
	uint	    nIds, nExIds, i;
	uint		dwCPUSpeed = MeasureCpuSpeed();

	qboolean    SSE3 = qfalse;
	qboolean	SSE4 = qfalse;
	qboolean	SSE41 = qfalse;
	qboolean	SSE42 = qfalse;
	qboolean	SSE2 = qfalse;
	qboolean	SSE = qfalse;
	qboolean	MMX = qfalse;
	qboolean	HTT = qfalse;
	qboolean	SMT = qfalse;
	qboolean	EM64T = qfalse;
	qboolean	AVX = qfalse;
	qboolean	AVX2 = qfalse;

	// __cpuid with an InfoType argument of 0 returns the number of
	// valid Ids in CPUInfo[0] and the CPU identification string in
	// the other three array elements. The CPU identification string is
	// not in linear order. The code below arranges the information 
	// in a human readable form.
	__cpuid(CPUInfo, 0);
	nIds = CPUInfo[0];
	memset(CPUString, 0, sizeof(CPUString));

	*((int*)CPUString) = CPUInfo[1];
	*((int*)(CPUString + 4)) = CPUInfo[3];
	*((int*)(CPUString + 8)) = CPUInfo[2];

	// Get the information associated with each valid Id
	for (i = 0; i <= nIds; ++i) {

		__cpuid(CPUInfo, i);

		// Interpret CPU feature information.
		if (i == 1)
		{
			SSE2 = (CPUInfo[3] & BIT(26));
			SSE3 = (CPUInfo[2] & BIT(0));
			SSE4 = (CPUInfo[2] & BIT(9));
			SSE41 = (CPUInfo[2] & BIT(19));
			SSE42 = (CPUInfo[2] & BIT(20));
			SSE = (CPUInfo[3] & BIT(25));
			MMX = (CPUInfo[3] & BIT(23));
			EM64T = (CPUInfo[3] & BIT(29));
			AVX = (CPUInfo[2] & BIT(28));
			AVX2 = (CPUInfo[1] & BIT(5));
			nFeatureInfo = CPUInfo[3];
		}
	}

	if (nIds >= 0x00000007) {
		__cpuid(CPUInfo, 0x00000007);
		AVX2 = (CPUInfo[1] & BIT(5));
	}

	// Calling __cpuid with 0x80000000 as the InfoType argument
	// gets the number of valid extended IDs.
	__cpuid(CPUInfo, 0x80000000);
	nExIds = CPUInfo[0];
	memset(CPUBrandString, 0, sizeof(CPUBrandString));

	// Get the information associated with each extended ID.
	for (i = 0x80000000; i <= nExIds; ++i) {

		__cpuid(CPUInfo, i);

		// Interpret CPU brand string.
		if (i == 0x80000002)
			memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
		else
			if (i == 0x80000003)
				memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
			else
				if (i == 0x80000004)
					memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
	}

	if (nIds >= 1) {

		if (nFeatureInfo) {

			Com_Printf("Cpu Brand Name: "S_COLOR_GREEN"%s\n", &CPUBrandString[0]);

			int  count = 0;
			char *find = strtok(&CPUBrandString[0], " ");

			while (find != NULL)
			{
				// print current line
				//	Com_Printf("%s\n", find);
				if (!Q_strcasecmp(find, "Ryzen"))
					count++;
				if (!Q_strcasecmp(find, "5"))
					count++;
				if (!Q_strcasecmp(find, "7"))
					count++;
				// select new line
				find = strtok(NULL, " ");
			}
			if (count >= 2)
				SMT = GetCpuCoresThreads(qtrue);
			else
				HTT = GetCpuCoresThreads(qfalse);

			float GHz = (float)dwCPUSpeed * 0.001;
			Com_Printf("CPU Speed: ~"S_COLOR_GREEN"%.3f"S_COLOR_WHITE" GHz\n", GHz);
			Com_Printf("Supported Extensions: ");

			__cpuid(CPUInfo, 0x80000001);

			if (SSE)
				Com_Printf(S_COLOR_YELLOW"SSE ");
			if (SSE2)
				Com_Printf(S_COLOR_YELLOW"SSE2 ");
			if (SSE3)
				Com_Printf(S_COLOR_YELLOW"SSE3 ");
			if (SSE4)
				Com_Printf(S_COLOR_YELLOW"SSE4 ");
			if (SSE41)
				Com_Printf(S_COLOR_YELLOW"SSE4.1 ");
			if (SSE42)
				Com_Printf(S_COLOR_YELLOW"SSE4.2 ");
			if (AVX)
				Com_Printf(S_COLOR_YELLOW"AVX ");
			if (AVX2)
				Com_Printf(S_COLOR_YELLOW"AVX2 ");
			if (HTT)
				Com_Printf(S_COLOR_YELLOW"HTT ");
			if (SMT)
				Com_Printf(S_COLOR_YELLOW"SMT ");
			if (EM64T)
				Com_Printf(S_COLOR_YELLOW"EM64T");
			Com_Printf("\n");

		}
	}

}

void Sys_GetMemorySize() {

	MEMORYSTATUSEX		ram;
	
	ram.dwLength = sizeof(ram);
	GlobalMemoryStatusEx(&ram);	
	
	DWORD physRam = ram.ullTotalPhys >> 20;

	Con_Printf(PRINT_ALL, "\n");
	Com_Printf("Phisical RAM:  "S_COLOR_GREEN"%d"S_COLOR_WHITE" GB\n", (physRam + 512) >>10);
	Com_Printf("Memory loaded: "S_COLOR_GREEN"%d"S_COLOR_WHITE" %%\n", ram.dwMemoryLoad);
	Con_Printf(PRINT_ALL, "\n");
}

BOOL Is64BitWindows() {
	BOOL f64 = FALSE;
	return IsWow64Process(GetCurrentProcess(), &f64) && f64;
}

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL(WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

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

qboolean Sys_CheckWindowsVersion() {

	RTL_OSVERSIONINFOEXW    rtl_OsVer;
	DWORD					prType;
	PGPI					pGPI;
	char					S[64], S2[64];

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

			// Get windows 10 OS number 
			DWORD	dwType = REG_SZ;
			HKEY	regKey = HKEY_LOCAL_MACHINE;
			HKEY	hKey = 0;
			cchar	*subkey = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
			char	sz_val[5];
			DWORD	len = 5;
			int		ver;

			if (RegOpenKey(regKey, subkey, &hKey) == ERROR_SUCCESS) {
				RegQueryValueEx(hKey, "ReleaseId", NULL, &dwType, (LPBYTE)&sz_val, &len);
				ver = atoi(sz_val);

				if (ver == 1507)
					sprintf(S2, "\n    'Threshold 1' " S_COLOR_WHITE "(" S_COLOR_GREEN "1507" S_COLOR_WHITE ")");
				else
					if (ver == 1511)
						sprintf(S2, "\n    'November Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "1511" S_COLOR_WHITE ")");
					else
						if (ver == 1607)
							sprintf(S2, "\n    'Anniversary Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "1607" S_COLOR_WHITE ")");
						else
							if (ver == 1703)
								sprintf(S2, "\n    'Creators Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "1703" S_COLOR_WHITE ")");
							else
								if (ver == 1709)
									sprintf(S2, "\n    'Fall Creators Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "1709" S_COLOR_WHITE ")");
								else
									if (ver == 1803)
										sprintf(S2, "\n    'April 2018 Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "1803" S_COLOR_WHITE ")");
									else
										if (ver == 1809)
											sprintf(S2, "\n    'October 2018 Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "1809" S_COLOR_WHITE ")");
										else
											sprintf(S2, "\n    'Unknow Update' " S_COLOR_WHITE "(" S_COLOR_GREEN "%i" S_COLOR_WHITE ")", ver);

				RegCloseKey(hKey);
			}

			if (!Is64BitWindows()) {

				if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
					Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 10 "S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, S2, rtl_OsVer.dwBuildNumber);
				else
					Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2016"S_COLOR_GREEN"x32 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, S2, rtl_OsVer.dwBuildNumber);
			}
			else {

				if (rtl_OsVer.wProductType == VER_NT_WORKSTATION)
					Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows 10 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, S2, rtl_OsVer.dwBuildNumber);
				else
					Com_Printf(S_COLOR_WHITE"OS: "S_COLOR_YELLOW"Microsoft Windows Server 2016 "S_COLOR_GREEN"x64 "S_COLOR_WHITE"%s"S_COLOR_YELLOW" %s "S_COLOR_WHITE"build "S_COLOR_GREEN"%d\n", S, S2, rtl_OsVer.dwBuildNumber);
			}

		}
}
return qtrue;
	
		return qfalse;
}