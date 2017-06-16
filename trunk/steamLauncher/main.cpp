// steam hack.cpp : Defines the entry point for the console application.
//

#include "windows.h"

#pragma warning(disable : 4996)		// vs 2005 security warnings

wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int main(int argc, char *argv[])
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation;

	StartupInfo.cb = sizeof(STARTUPINFO);
	StartupInfo.lpReserved = NULL;
	StartupInfo.lpDesktop = NULL;
	StartupInfo.lpTitle = NULL;
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
	StartupInfo.wShowWindow = SW_SHOWNORMAL;
	StartupInfo.cbReserved2 = 0;
	StartupInfo.lpReserved2 = NULL;


	char cmd[256];
	strcpy(cmd, "quake2xp");
	strcat(cmd, " ");

	for (int i = 1; i < argc; i++)
	{
		strcat(cmd, argv[i]);
		strcat(cmd, " ");
	}
		
	LPTSTR szCmdline = convertCharArrayToLPCWSTR(cmd);

	CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);
	
	return 0;
}

