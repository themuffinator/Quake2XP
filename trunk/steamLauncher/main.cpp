/*==============================================
quake2xp steam(tm) launcher
console version support unicode and non unicode
win32 support non unicode only
==============================================*/

#include "windows.h"


#ifdef UNICODE
wchar_t *convertCharArrayToLPTSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

/*
// TODO !!! LPTSTR to CHAR
LPWSTR pwz = "some text";
char* psz = new char [wcslen(pwsz) + 1];

wsprintfA ( psz, "%S", pwsz);
*/
#endif

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
strcpy_s(cmd, "quake2xp");
strcat_s(cmd, " ");

for (int i = 1; i < argc; i++)
{
strcat_s(cmd, argv[i]);
strcat_s(cmd, " ");
}

#ifdef UNICODE

LPTSTR szCmdline = convertCharArrayToLPTSTR(cmd);
CreateProcess(NULL, szCmdline, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

#else

CreateProcess(NULL, (LPSTR)cmd, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);

#endif

return 0;
}

// win32 version non unicode only
#ifndef CONSOLE

#ifndef UNICODE 

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	if (hPrevInstance)
		return 0;

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
	strcpy_s(cmd, "quake2xp");
	strcat_s(cmd, " ");
	strcat_s(cmd, lpCmdLine);

	CreateProcess(NULL, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &StartupInfo, &ProcessInformation);


	return 1;
}

#endif
#endif
