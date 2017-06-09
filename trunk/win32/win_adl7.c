// Ati Driver SDK
// Overdrive N 

#include "../ref_gl/r_local.h"
#include "adl/adl_sdk.h"

// Definitions of the used function pointers. Add more if you use other ADL APIs
typedef int(*ADL_MAIN_CONTROL_CREATE)(ADL_MAIN_MALLOC_CALLBACK, int);
typedef int(*ADL_MAIN_CONTROL_DESTROY)();
typedef int(*ADL_FLUSH_DRIVER_DATA)(int);
typedef int(*ADL2_ADAPTER_ACTIVE_GET) (ADL_CONTEXT_HANDLE, int, int*);

typedef int(*ADL_ADAPTER_NUMBEROFADAPTERS_GET) (int*);
typedef int(*ADL_ADAPTER_ADAPTERINFO_GET) (LPAdapterInfo, int);
typedef int(*ADL_ADAPTERX2_CAPS) (int, int*);
typedef int(*ADL2_OVERDRIVE_CAPS) (ADL_CONTEXT_HANDLE context, int iAdapterIndex, int * iSupported, int * iEnabled, int * iVersion);
typedef int(*ADL2_OVERDRIVEN_CAPABILITIES_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNCapabilities*);
typedef int(*ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)	(ADL_CONTEXT_HANDLE, int, ADLODNPerformanceLevels*);
typedef int(*ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPerformanceStatus*);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_GET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_FANCONTROL_SET) (ADL_CONTEXT_HANDLE, int, ADLODNFanControl*);
typedef int(*ADL2_OVERDRIVEN_POWERLIMIT_GET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int(*ADL2_OVERDRIVEN_POWERLIMIT_SET) (ADL_CONTEXT_HANDLE, int, ADLODNPowerLimitSetting*);
typedef int(*ADL2_OVERDRIVEN_TEMPERATURE_GET) (ADL_CONTEXT_HANDLE, int, int, int*);
HINSTANCE hDLL;

ADL_MAIN_CONTROL_CREATE          ADL_Main_Control_Create = NULL;
ADL_MAIN_CONTROL_DESTROY         ADL_Main_Control_Destroy = NULL;
ADL_ADAPTER_NUMBEROFADAPTERS_GET ADL_Adapter_NumberOfAdapters_Get = NULL;
ADL_ADAPTER_ADAPTERINFO_GET      ADL_Adapter_AdapterInfo_Get = NULL;
ADL_ADAPTERX2_CAPS ADL_AdapterX2_Caps = NULL;
ADL2_ADAPTER_ACTIVE_GET				ADL2_Adapter_Active_Get = NULL;
ADL2_OVERDRIVEN_CAPABILITIES_GET ADL2_OverdriveN_Capabilities_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET ADL2_OverdriveN_SystemClocks_Get = NULL;
ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET ADL2_OverdriveN_SystemClocks_Set = NULL;
ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET ADL2_OverdriveN_PerformanceStatus_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_GET ADL2_OverdriveN_FanControl_Get = NULL;
ADL2_OVERDRIVEN_FANCONTROL_SET ADL2_OverdriveN_FanControl_Set = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_GET ADL2_OverdriveN_PowerLimit_Get = NULL;
ADL2_OVERDRIVEN_POWERLIMIT_SET ADL2_OverdriveN_PowerLimit_Set = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_OverdriveN_MemoryClocks_Get = NULL;
ADL2_OVERDRIVEN_MEMORYCLOCKS_GET ADL2_OverdriveN_MemoryClocks_Set = NULL;
ADL2_OVERDRIVE_CAPS ADL2_Overdrive_Caps = NULL;
ADL2_OVERDRIVEN_TEMPERATURE_GET ADL2_OverdriveN_Temperature_Get = NULL;


// Memory allocation function
void* __stdcall ADL_Main_Memory_Alloc(int iSize)
{
	void* lpBuffer = malloc(iSize);
	return lpBuffer;
}

// Optional Memory de-allocation function
void __stdcall ADL_Main_Memory_Free(void** lpBuffer)
{
	if (NULL != *lpBuffer)
	{
		free(*lpBuffer);
		*lpBuffer = NULL;
	}
}

ADL_CONTEXT_HANDLE context = NULL;

LPAdapterInfo   lpAdapterInfo = NULL;
int  iNumberAdapters;

qboolean GLimp_InitADL()
{
	Com_Printf("" S_COLOR_YELLOW "\n...Initializing ATI Driver SDK : ");
	// Load the ADL dll
	hDLL = LoadLibrary("atiadlxx.dll");
	if (hDLL == NULL)
	{
		// A 32 bit calling application on 64 bit OS will fail to LoadLibrary.
		// Try to load the 32 bit library (atiadlxy.dll) instead
		hDLL = LoadLibrary("atiadlxy.dll");
	}

	if (NULL == hDLL)
	{
		Com_Printf(S_COLOR_MAGENTA"ADL library not found\n");
		return qfalse;
	}

	ADL_Main_Control_Create = (ADL_MAIN_CONTROL_CREATE)GetProcAddress(hDLL, "ADL_Main_Control_Create");
	ADL_Main_Control_Destroy = (ADL_MAIN_CONTROL_DESTROY)GetProcAddress(hDLL, "ADL_Main_Control_Destroy");
	ADL_Adapter_NumberOfAdapters_Get = (ADL_ADAPTER_NUMBEROFADAPTERS_GET)GetProcAddress(hDLL, "ADL_Adapter_NumberOfAdapters_Get");
	ADL_Adapter_AdapterInfo_Get = (ADL_ADAPTER_ADAPTERINFO_GET)GetProcAddress(hDLL, "ADL_Adapter_AdapterInfo_Get");
	ADL_AdapterX2_Caps = (ADL_ADAPTERX2_CAPS)GetProcAddress(hDLL, "ADL_AdapterX2_Caps");
	ADL2_Adapter_Active_Get = (ADL2_ADAPTER_ACTIVE_GET)GetProcAddress(hDLL, "ADL2_Adapter_Active_Get");
	ADL2_OverdriveN_Capabilities_Get = (ADL2_OVERDRIVEN_CAPABILITIES_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_Capabilities_Get");
	ADL2_OverdriveN_SystemClocks_Get = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_SystemClocks_Get");
	ADL2_OverdriveN_SystemClocks_Set = (ADL2_OVERDRIVEN_SYSTEMCLOCKS_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_SystemClocks_Set");
	ADL2_OverdriveN_MemoryClocks_Get = (ADL2_OVERDRIVEN_MEMORYCLOCKS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_MemoryClocks_Get");
	ADL2_OverdriveN_MemoryClocks_Set = (ADL2_OVERDRIVEN_MEMORYCLOCKS_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_MemoryClocks_Set");
	ADL2_OverdriveN_PerformanceStatus_Get = (ADL2_OVERDRIVEN_PERFORMANCESTATUS_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_PerformanceStatus_Get");
	ADL2_OverdriveN_FanControl_Get = (ADL2_OVERDRIVEN_FANCONTROL_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_FanControl_Get");
	ADL2_OverdriveN_FanControl_Set = (ADL2_OVERDRIVEN_FANCONTROL_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_FanControl_Set");
	ADL2_OverdriveN_PowerLimit_Get = (ADL2_OVERDRIVEN_POWERLIMIT_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_PowerLimit_Get");
	ADL2_OverdriveN_PowerLimit_Set = (ADL2_OVERDRIVEN_POWERLIMIT_SET)GetProcAddress(hDLL, "ADL2_OverdriveN_PowerLimit_Set");
	ADL2_OverdriveN_Temperature_Get = (ADL2_OVERDRIVEN_TEMPERATURE_GET)GetProcAddress(hDLL, "ADL2_OverdriveN_Temperature_Get");
	ADL2_Overdrive_Caps = (ADL2_OVERDRIVE_CAPS)GetProcAddress(hDLL, "ADL2_Overdrive_Caps");
	if (NULL == ADL_Main_Control_Create ||
		NULL == ADL_Main_Control_Destroy ||
		NULL == ADL_Adapter_NumberOfAdapters_Get ||
		NULL == ADL_Adapter_AdapterInfo_Get ||
		NULL == ADL_AdapterX2_Caps ||
		NULL == ADL2_Adapter_Active_Get ||
		NULL == ADL2_OverdriveN_Capabilities_Get ||
		NULL == ADL2_OverdriveN_SystemClocks_Get ||
		NULL == ADL2_OverdriveN_SystemClocks_Set ||
		NULL == ADL2_OverdriveN_MemoryClocks_Get ||
		NULL == ADL2_OverdriveN_MemoryClocks_Set ||
		NULL == ADL2_OverdriveN_PerformanceStatus_Get ||
		NULL == ADL2_OverdriveN_FanControl_Get ||
		NULL == ADL2_OverdriveN_FanControl_Set ||
		NULL == ADL2_Overdrive_Caps
		)
	{
		Com_Printf(S_COLOR_RED"Failed to get ADL function pointers\n");
		return qfalse;
	}

	if (ADL_OK != ADL_Main_Control_Create(ADL_Main_Memory_Alloc, 1))
	{
		Com_Printf(S_COLOR_RED"Failed to initialize nested ADL2 context");
		return ADL_ERR;
	}
	Com_Printf(S_COLOR_GREEN"ok.\n");
	return qtrue;
}

void ADL_unload(){

	ADL_Main_Control_Destroy();
	FreeLibrary(hDLL);
}

void adl_PrintGpuPerformance()
{
	int		i, active = 0;
	int		phisAdp;

	int iSupported, iEnabled, iVersion;

	// Obtain the number of adapters for the system
	if (ADL_OK != ADL_Adapter_NumberOfAdapters_Get(&iNumberAdapters))
	{
		Com_Printf(S_COLOR_RED"Cannot get the number of adapters!\n");
		return;
	}

	if (iNumberAdapters > 0)
	{
		lpAdapterInfo = (LPAdapterInfo)malloc(sizeof(AdapterInfo) * iNumberAdapters);
		memset(lpAdapterInfo, '\0', sizeof(AdapterInfo) * iNumberAdapters);

		// Get the AdapterInfo structure for all adapters in the system
		ADL_Adapter_AdapterInfo_Get(lpAdapterInfo, sizeof(AdapterInfo) * iNumberAdapters);
	}

	// no crossfire config...

	phisAdp = iNumberAdapters + 1;

//	find num of phisical adapters
	for (i = 0; i < iNumberAdapters; i++)
	{
		if (lpAdapterInfo[i].iBusNumber > -1)
		{
			phisAdp--;
		}
	}

	// Repeat for all available adapters in the system
	for (i = 0; i < phisAdp; i++)
	{
		if (lpAdapterInfo[i].iBusNumber > -1)
		{
			ADL2_Overdrive_Caps(context, lpAdapterInfo[i].iAdapterIndex, &iSupported, &iEnabled, &iVersion);

			if (iVersion == 7)
			{
				// found! 
// gpu start

				ADLODNCapabilities overdriveCapabilities;
				memset(&overdriveCapabilities, 0, sizeof(ADLODNCapabilities));

				if (ADL_OK != ADL2_OverdriveN_Capabilities_Get(context, lpAdapterInfo[i].iAdapterIndex, &overdriveCapabilities))
				{
					Com_Printf("ADL2_OverdriveN_Capabilities_Get is failed\n");
				}
// temperature
					ADLODNPowerLimitSetting odNPowerControl;
				memset(&odNPowerControl, 0, sizeof(ADLODNPowerLimitSetting));

				if (ADL_OK != ADL2_OverdriveN_PowerLimit_Get(context, lpAdapterInfo[i].iAdapterIndex, &odNPowerControl))
				{
					Com_Printf("ADL2_OverdriveN_PowerLimit_Get is failed\n");
				}
				else
				{
					int temp;
					ADL2_OverdriveN_Temperature_Get(context, lpAdapterInfo[i].iAdapterIndex, 1, &temp);

					Com_Printf("Current temperature : %d\n", temp >> 10);
					Com_Printf("-------------------------------------------------\n");

				}
// fan speed
				ADLODNFanControl odNFanControl;
				memset(&odNFanControl, 0, sizeof(ADLODNFanControl));

				if (ADL_OK != ADL2_OverdriveN_FanControl_Get(context, lpAdapterInfo[i].iAdapterIndex, &odNFanControl))
				{
					Com_Printf("ADL2_OverdriveN_FanControl_Get is failed\n");
				}
				else
				{
					Com_Printf("---------Fan Current Speed--------------\n");
					Com_Printf("Current Fan Speed : %d\n", odNFanControl.iCurrentFanSpeed);
					Com_Printf("Current Fan Speed Mode : %d\n", odNFanControl.iCurrentFanSpeedMode);
					Com_Printf("-----------------------------------------\n");

				}
				ADLODNPerformanceStatus odNPerformanceStatus;
				memset(&odNPerformanceStatus, 0, sizeof(ADLODNPerformanceStatus));

				if (ADL_OK != ADL2_OverdriveN_PerformanceStatus_Get(context, lpAdapterInfo[i].iAdapterIndex, &odNPerformanceStatus))
				{
					Com_Printf("ADL2_OverdriveN_PerformanceStatus_Get is failed\n");
				}
				else
				{
					Com_Printf("-------------------------------------------------\n");
					Com_Printf("ADL2_OverdriveN_PerformanceStatus_Get Data\n");
					Com_Printf("-------------------------------------------------\n");
					// fix!!!
					Com_Printf("CoreClock : %d\n", odNPerformanceStatus.iCoreClock );
					Com_Printf("CurrentBusSpeed : %d\n", odNPerformanceStatus.iCurrentBusSpeed);
					Com_Printf("MemoryClock : %d\n", odNPerformanceStatus.iMemoryClock);
					Com_Printf("-------------------------------------------------\n");

				}

//========gpu end
			}
		}
	}

}
