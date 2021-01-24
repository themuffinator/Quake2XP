/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

// NV API SDK

#include "../ref_gl/r_local.h"

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
	
	Com_Printf("\n==================================\n\n");

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

extern qboolean adlInit;

void R_GpuInfo_f(void) {

	NvAPI_Status		ret = NVAPI_OK;
	NvAPI_ShortString	string;

	if (adlInit) {
		ADL_PrintGpuInfo();
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
		Com_Printf("...Get GPU statistic from: " S_COLOR_GREEN "%s\n", string);

	ret = NvAPI_GPU_GetVbiosVersionString(hPhysicalGpu[0], string);
	if (ret != NVAPI_OK) {
		NvAPI_GetErrorMessage(ret, string);
		Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetVbiosVersionString() fail: %\n", string);
	}
	else
		Com_Printf("...Bios Version: " S_COLOR_GREEN "%s\n", string);
	
	for (int i = 0; i < physicalGpuCount; i++) {

		Com_Printf("\n   GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE ":\n", i);
				
		// get gpu temperature
		NV_GPU_THERMAL_SETTINGS	thermal;
		thermal.version = NV_GPU_THERMAL_SETTINGS_VER_2;
		ret = NvAPI_GPU_GetThermalSettings(hPhysicalGpu[i], 0, &thermal);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetThermalSettings() fail: %\n", string);
		}
		else
			Com_Printf("...temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius (%s)\n", thermal.sensor[i].currentTemp,
				GLimp_NvApi_GetThermalController(thermal.sensor[i].controller));

		// get fans speed
		NvU32 rpm = 0;
		ret = NvAPI_GPU_GetTachReading(hPhysicalGpu[i], &rpm);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetTachReading() fail: %s\n", string);
		}
		else
			Com_Printf("...fan speed: " S_COLOR_GREEN "%u" S_COLOR_WHITE " rpm\n", rpm);

		NvU32 pWidth;
		uint pcieLines = 0;
		ret = NvAPI_GPU_GetCurrentPCIEDownstreamWidth(hPhysicalGpu[i], &pWidth);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetCurrentPCIEDownstreamWidth() fail: %\n", string);
		}
		else
			pcieLines = (uint)pWidth;

		NV_GPU_DYNAMIC_PSTATES_INFO_EX	m_DynamicPStateInfo;
		m_DynamicPStateInfo.version = NV_GPU_DYNAMIC_PSTATES_INFO_EX_VER;
		ret = NvAPI_GPU_GetDynamicPstatesInfoEx(hPhysicalGpu[i], &m_DynamicPStateInfo);
		if (ret != NVAPI_OK){
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetDynamicPstatesInfoEx() fail: %s", string);
		}
		else
			Com_Printf("...utilization: " S_COLOR_YELLOW "Core " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%, " S_COLOR_YELLOW "Frame Buffer " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%, " S_COLOR_YELLOW "PCIe x%i Bus " S_COLOR_GREEN "%u" S_COLOR_WHITE " %%\n",
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_GPU].percentage,
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_FB].percentage,
				pcieLines,
				m_DynamicPStateInfo.utilization[NV_UTIL_DOMAIN_BUS].percentage);
		
		// get gpu & vram frequencies
		NV_GPU_CLOCK_FREQUENCIES clocks;
		clocks.version = NV_GPU_CLOCK_FREQUENCIES_VER;
		clocks.ClockType = NV_GPU_CLOCK_FREQUENCIES_CURRENT_FREQ;
		ret = NvAPI_GPU_GetAllClockFrequencies(hPhysicalGpu[i], &clocks);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetAllClockFrequencies() fail: %s\n", string);
		}
		else {
			if (clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].bIsPresent && clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].bIsPresent)
				Com_Printf("...frequencies: " S_COLOR_YELLOW "GPU: " S_COLOR_GREEN "%u" S_COLOR_WHITE " MHz " S_COLOR_YELLOW "VRAM: " S_COLOR_GREEN "%u" S_COLOR_WHITE " MHz\n",
					(NvU32)((clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_GRAPHICS].frequency + 500) / 1000),
					(NvU32)((clocks.domain[NVAPI_GPU_PUBLIC_CLOCK_MEMORY].frequency + 500) / 1000));
		}

		// get performance states infos
		NvU32  inputFlags = 1U;
		NV_GPU_PERF_PSTATES_INFO PerfPstatesInfo;
		PerfPstatesInfo.version = NV_GPU_PERF_PSTATES_INFO_VER;
		ret = NvAPI_GPU_GetPstatesInfoEx(hPhysicalGpu[i], &PerfPstatesInfo, inputFlags);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetCurrentPstate() fail: %s\n", string);
		}
		else
			Com_Printf("\n...GPU Performance States: " S_COLOR_GREEN "%i\n", (int)PerfPstatesInfo.numPstates);
		
		NV_GPU_PERF_PSTATE_ID currentPState;
		ret = NvAPI_GPU_GetCurrentPstate(hPhysicalGpu[i], &currentPState);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetCurrentPstate() fail: %s\n", string);
		}
		else
			Com_Printf("...Current GPU Performance Level: " S_COLOR_GREEN "%i\n", (int)currentPState);

		// perf states speeds
		NV_GPU_PERF_PSTATES20_INFO pStates2info = {0};
		pStates2info.version = MAKE_NVAPI_VERSION(pStates2info, 2);
		ret = NvAPI_GPU_GetPstates20(hPhysicalGpu[i], &pStates2info);
		if (NVAPI_OK != ret) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetPstates20() fail: %s\n", string);
		}
		else{		
			for (int z = 0; z < pStates2info.numPstates; z++) {

				Com_Printf(S_COLOR_YELLOW"%s" S_COLOR_WHITE "P" S_COLOR_GREEN "%i" S_COLOR_WHITE ": " S_COLOR_YELLOW "GPU " S_COLOR_GREEN "%i" S_COLOR_WHITE "-" S_COLOR_GREEN "%i" S_COLOR_WHITE " MHz " S_COLOR_YELLOW "VRAM " S_COLOR_GREEN "%i" S_COLOR_WHITE "-" S_COLOR_GREEN "%i" S_COLOR_WHITE " MHz\n",
					pStates2info.pstates[z].pstateId == currentPState ? ">" : " ",
					(int)pStates2info.pstates[z].pstateId,
					pStates2info.pstates[z].clocks[0].data.range.minFreq_kHz / 1000, //gpu
					pStates2info.pstates[z].clocks[0].data.range.maxFreq_kHz / 1000,
					pStates2info.pstates[z].clocks[1].data.range.minFreq_kHz / 1000, //vram
					pStates2info.pstates[z].clocks[1].data.range.maxFreq_kHz / 1000);
			}
		
		}

		NV_DISPLAY_DRIVER_MEMORY_INFO gpuMemoryStatus;
		gpuMemoryStatus.version = MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V2, 2);
		ret = NvAPI_GPU_GetMemoryInfo(hPhysicalGpu[i], (NV_DISPLAY_DRIVER_MEMORY_INFO *)&gpuMemoryStatus);
		if (NVAPI_OK != ret) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetMemoryInfo() fail: %s\n", string);
		} else{
			float used = (float)gpuMemoryStatus.dedicatedVideoMemory - (float)gpuMemoryStatus.curAvailableDedicatedVideoMemory;
			float total = (float)gpuMemoryStatus.dedicatedVideoMemory;
			
			total /= (1024 * 1024);
			used /= (1024 * 1024);
			
			Com_Printf("\n...Full GPU Memory: " S_COLOR_GREEN "%.2f" S_COLOR_WHITE " Gb\n", total);
			Com_Printf("...Used GPU Memory: " S_COLOR_GREEN "%.2f" S_COLOR_WHITE " Gb\n", used);
		}

		Com_Printf("\n==========================================================\n");
	}
}
