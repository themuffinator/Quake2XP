/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

// NV API SDK

#include "../renderer/r_local.h"


NvPhysicalGpuHandle hPhysicalGpu[NVAPI_MAX_PHYSICAL_GPUS];
NvU32 physicalGpuCount = 0;
uint32_t nvmlPhysicalGpuCount = 0;

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
void NvApi_GetDisplayInfo() {
	NvAPI_Status status = NVAPI_OK;
	NvU32 deviceCount = 0;
	NV_DISPLAYCONFIG_PATH_INFO_V2* pathInfo = NULL;

	Com_Printf("\n...collecting info from connected displays.\n");
	status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);

	if ((status == NVAPI_OK) && (deviceCount > 0)) {

		pathInfo = malloc(deviceCount * (sizeof(NV_DISPLAYCONFIG_PATH_INFO_V2)));

		for (int i = 0; i < deviceCount; i++){
			pathInfo[i].targetInfo = 0;
			pathInfo[i].targetInfoCount = 0;
			pathInfo[i].version = NV_DISPLAYCONFIG_PATH_INFO_VER2;
			pathInfo[i].sourceModeInfo = 0;
			pathInfo[i].reserved = 0;
		}

		status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
		if (status == NVAPI_OK) {
			for (int i = 0; i < deviceCount; i++)
			{
				pathInfo[i].sourceModeInfo = malloc(sizeof(NV_DISPLAYCONFIG_SOURCE_MODE_INFO_V1));
				pathInfo[i].sourceModeInfo->reserved = 0;
				pathInfo[i].targetInfo = malloc(pathInfo[i].targetInfoCount * sizeof(NV_DISPLAYCONFIG_PATH_TARGET_INFO_V2));
				for (int j = 0; j < pathInfo[i].targetInfoCount; j++) {
					pathInfo[i].targetInfo[j].details = malloc(sizeof(NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_V1));
					pathInfo[i].targetInfo[j].details->version = NV_DISPLAYCONFIG_PATH_ADVANCED_TARGET_INFO_VER1;
					pathInfo[i].targetInfo[j].details->reserved = 0;
				}
			}
		}
		
		status = NvAPI_DISP_GetDisplayConfig(&deviceCount, pathInfo);
		if (status == NVAPI_OK) {
			//--------------
			for (int i = 0; i < deviceCount; i++)
			{
				for (int j = 0; j < pathInfo[i].targetInfoCount; j++) {

					Com_Printf("\n>%i:" S_COLOR_GREEN "%s\n",i, pathInfo[i].targetInfo[j].details->timing.etc.name);
					Com_Printf("Connection Type: ");
					switch (pathInfo[i].targetInfo[j].details->connector)
					{
					case NVAPI_GPU_CONNECTOR_DVI_I:
						Com_Printf(S_COLOR_GREEN"DVI I\n");
						break;
					case NVAPI_GPU_CONNECTOR_DVI_D:
						Com_Printf(S_COLOR_GREEN"DVI D\n");
						break;
					case NVAPI_GPU_CONNECTOR_LFH_DVI_I_1:
						Com_Printf(S_COLOR_GREEN"LFH DVI I 1\n");
						break;
					case NVAPI_GPU_CONNECTOR_LFH_DVI_I_2:
						Com_Printf(S_COLOR_GREEN"LFH DVI I 2\n");
						break;
					case NVAPI_GPU_CONNECTOR_DISPLAYPORT_EXTERNAL:
						Com_Printf(S_COLOR_GREEN"DISPLAYPORT EXT\n");
						break;
					case NVAPI_GPU_CONNECTOR_DISPLAYPORT_INTERNAL:
						Com_Printf(S_COLOR_GREEN"DISPLAYPORT INT\n");
						break;
					case NVAPI_GPU_CONNECTOR_DISPLAYPORT_MINI_EXT:
						Com_Printf(S_COLOR_GREEN"DISPLAYPORT MINI EXT\n");
						break;
					case NVAPI_GPU_CONNECTOR_HDMI_A:
						Com_Printf(S_COLOR_GREEN"HDMI A\n");
						break;
					case NVAPI_GPU_CONNECTOR_HDMI_C_MINI:
						Com_Printf(S_COLOR_GREEN"HDMI C MINI\n");
						break;
					case NVAPI_GPU_CONNECTOR_LFH_DISPLAYPORT_1:
						Com_Printf(S_COLOR_GREEN"LFH DISPLAYPORT 1\n");
						break;
					case NVAPI_GPU_CONNECTOR_LFH_DISPLAYPORT_2:
						Com_Printf(S_COLOR_GREEN"LFH DISPLAYPORT 2\n");
						break;
					case NVAPI_GPU_CONNECTOR_USB_C:
						Com_Printf(S_COLOR_GREEN"USB C\n");
						break;
					case NVAPI_GPU_CONNECTOR_UNKNOWN:
						Com_Printf(S_COLOR_MAGENTA"UNKNOW\n");
						break;
					default:
						Com_Printf(S_COLOR_MAGENTA"UNKNOW\n");
						break;
					}
					if(pathInfo[i].sourceModeInfo->colorFormat == NV_FORMAT_A16B16G16R16F)
						Com_Printf("30bit Display: " S_COLOR_GREEN "true\n");
					else
						Com_Printf("30bit Display: " S_COLOR_GREEN "false\n");

					Com_Printf("Scale Mode: ");
					switch (pathInfo[i].targetInfo[j].details->scaling)
					{
					case NV_SCALING_DEFAULT:
						Com_Printf(S_COLOR_GREEN"Default\n");
						break;
					case NV_SCALING_GPU_SCALING_TO_CLOSEST:
						Com_Printf(S_COLOR_GREEN"Balanced  - Full Screen\n");
						break;
					case NV_SCALING_GPU_SCALING_TO_NATIVE:
						Com_Printf(S_COLOR_GREEN"GPU - Full Screen\n");
						break;
					case NV_SCALING_GPU_SCANOUT_TO_NATIVE:
						Com_Printf(S_COLOR_GREEN"GPU - Centered|No Scaling\n");
						break;
					case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_NATIVE:
						Com_Printf(S_COLOR_GREEN"GPU - Aspect Ratio\n");
						break;
					case NV_SCALING_GPU_SCALING_TO_ASPECT_SCANOUT_TO_CLOSEST:
						Com_Printf(S_COLOR_GREEN"Balanced  - Aspect Ratio\n");
						break;
					case NV_SCALING_GPU_SCANOUT_TO_CLOSEST:
						Com_Printf(S_COLOR_GREEN"Balanced  - Centered|No Scaling\n");
						break;
					case NV_SCALING_GPU_INTEGER_ASPECT_SCALING:
						Com_Printf(S_COLOR_GREEN"Force GPU - Integer Scaling\n");
					default:
						Com_Printf(S_COLOR_GREEN"Nothing\n");
						break;
					}
				}
			}
			
		}
	}
}

typedef struct chroma_s{
	float red_x, red_y;
	float green_x, green_y;
	float blue_x, blue_y;
	float wp_x, wp_y;
}chroma_t;

const chroma_t chromaList[] = {
	{ 0.64000f, 0.33000f, 0.30000f, 0.60000f, 0.15000f, 0.06000f, 0.31270f, 0.32900f }, // rec709
	{ 0.68000f, 0.32000f, 0.26500f, 0.69000f, 0.15000f, 0.06000f, 0.31400f, 0.35100f }, // DCI-P3
	{ 0.70800f, 0.29200f, 0.17000f, 0.79700f, 0.13100f, 0.04600f, 0.31270f, 0.32900f }, // bt2020
	{ 0.68000f, 0.32000f, 0.26500f, 0.69000f, 0.15000f, 0.06000f, 0.32168f, 0.33767f }, // D60 DCI
	{ 0.64000f, 0.33000f, 0.21000f, 0.71000f, 0.15000f, 0.06000f, 0.31270f, 0.32900f }, // Adobe98
};

typedef enum {
	CS_REC709 = 0,
	CS_DCIP3 = 1,
	CS_BT2020 = 2,
	CS_P3D60 = 3,
	CS_ADOBE98 = 4,
	CS_INVALID = -1
} colorSpace;

void NvApi_SetUhdDisplays(bool enableHDR){

	NvAPI_Status ret = NVAPI_OK;
	NvAPI_ShortString string;
	NvU32 displayCount = 16, flags = 0;

	Com_Printf("\n...Looking For HDR Displays\n");

	for (NvU32 i = 0; i < physicalGpuCount; ++i){
		NV_GPU_DISPLAYIDS displayArray[16] = {0};
		displayArray[0].version = NV_GPU_DISPLAYIDS_VER;

		ret = NvAPI_GPU_GetConnectedDisplayIds(hPhysicalGpu[i], displayArray, &displayCount, flags);

		if (NVAPI_OK != ret){
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"NvAPI_GPU_GetConnectedDisplayIds() fail: %s\n", string);
			continue;
		}

		for (NvU32 j = 0; j < displayCount; ++j){
			NV_HDR_CAPABILITIES hdrCaps = {0};
			hdrCaps.version = NV_HDR_CAPABILITIES_VER;

			ret = NvAPI_Disp_GetHdrCapabilities(displayArray[j].displayId, &hdrCaps);

			if (ret != NVAPI_OK){
				NvAPI_GetErrorMessage(ret, string);
				Com_Printf(S_COLOR_RED"NvAPI_Disp_GetHdrCapabilities() fail: %s\n", string);
				continue;
			}				
				if (hdrCaps.isST2084EotfSupported){
					NV_HDR_COLOR_DATA hdrColorData = {0};
					memset(&hdrColorData, 0, sizeof(hdrColorData));

					hdrColorData.version = NV_HDR_COLOR_DATA_VER;
					hdrColorData.cmd = NV_HDR_CMD_SET;
					hdrColorData.static_metadata_descriptor_id = NV_STATIC_METADATA_TYPE_1;
					hdrColorData.hdrMode = enableHDR ? NV_HDR_MODE_UHDBD : NV_HDR_MODE_OFF;

					int cs = CS_BT2020;
					hdrColorData.mastering_display_data.displayPrimary_x0 = (NvU16)(chromaList[cs].red_x * 50000.0f);
					hdrColorData.mastering_display_data.displayPrimary_y0 = (NvU16)(chromaList[cs].red_y * 50000.0f);
					hdrColorData.mastering_display_data.displayPrimary_x1 = (NvU16)(chromaList[cs].green_x * 50000.0f);
					hdrColorData.mastering_display_data.displayPrimary_y1 = (NvU16)(chromaList[cs].green_y * 50000.0f);
					hdrColorData.mastering_display_data.displayPrimary_x2 = (NvU16)(chromaList[cs].blue_x * 50000.0f);
					hdrColorData.mastering_display_data.displayPrimary_y2 = (NvU16)(chromaList[cs].blue_y * 50000.0f);
					hdrColorData.mastering_display_data.displayWhitePoint_x = (NvU16)(chromaList[cs].wp_x * 50000.0f);
					hdrColorData.mastering_display_data.displayWhitePoint_y = (NvU16)(chromaList[cs].wp_y * 50000.0f);
					hdrColorData.mastering_display_data.max_content_light_level = 1000;
					hdrColorData.mastering_display_data.max_display_mastering_luminance = 1000;
					hdrColorData.mastering_display_data.max_frame_average_light_level = 100;
					hdrColorData.mastering_display_data.min_display_mastering_luminance = 1;

					if(enableHDR)
						Com_Printf(">%d:" S_COLOR_GREEN " Hdr Mode Enabled.\n", j);
					else
						Com_Printf(">%d:" S_COLOR_YELLOW " Hdr Mode Disabled.\n", j);

					ret = NvAPI_Disp_HdrColorControl(displayArray[j].displayId, &hdrColorData);

					if (ret != NVAPI_OK){
						NvAPI_GetErrorMessage(ret, string);
						Com_Printf(S_COLOR_RED"NvAPI_Disp_HdrColorControl() fail: %s\n", string);
						continue;
					}
					if(enableHDR)
						gl_config.hdrDisplay = true;
				}
				else {
					Com_Printf(">%d: " S_COLOR_MAGENTA "Don't Supported Hdr.\n", j);
					gl_config.hdrDisplay = false;
				}

		}
	}

}

typedef enum _NV_RAM_TYPE
{
	NV_RAM_TYPE_UNKNOWN = 0,
	NV_RAM_TYPE_SDRAM,
	NV_RAM_TYPE_DDR1,
	NV_RAM_TYPE_DDR2,
	NV_RAM_TYPE_GDDR2,
	NV_RAM_TYPE_GDDR3,
	NV_RAM_TYPE_GDDR4,
	NV_RAM_TYPE_DDR3,
	NV_RAM_TYPE_GDDR5,
	NV_RAM_TYPE_LPDDR2,
	NV_RAM_TYPE_GDDR5X,
	NV_RAM_TYPE_GDDR6 = 14,
	NV_RAM_TYPE_GDDR6X = 15
}NV_RAM_TYPE;

typedef enum _NV_RAM_MAKER
{
	NV_RAM_MAKER_NONE,
	NV_RAM_MAKER_SAMSUNG,
	NV_RAM_MAKER_QIMONDA,
	NV_RAM_MAKER_ELPIDA,
	NV_RAM_MAKER_ETRON,
	NV_RAM_MAKER_NANYA,
	NV_RAM_MAKER_HYNIX,
	NV_RAM_MAKER_MOSEL,
	NV_RAM_MAKER_WINBOND,
	NV_RAM_MAKER_ELITE,
	NV_RAM_MAKER_MICRON
}NV_RAM_MAKER;

HINSTANCE nv_hDLL;

typedef void *(*NvAPI_QueryInterface_t)			(unsigned int offset);
typedef int		(*NvAPI_GPU_GetRamType_t)		(int *handle, NV_RAM_TYPE *pRamType);
typedef int		(*NvAPI_GPU_GetRamMaker_t)		(int *handle, NV_RAM_MAKER *pRamMaker);
typedef int		(*NvAPI_GPU_GetRamBusWidth_t)	(NvPhysicalGpuHandle hPhysicalGpu, NvU32 *pBusWidth);

NvAPI_QueryInterface_t		NvAPI_GPU_QueryInterface	= NULL;
NvAPI_GPU_GetRamType_t		NvAPI_GPU_GetRamType		= NULL;
NvAPI_GPU_GetRamMaker_t		NvAPI_GPU_GetMemMarker		= NULL;
NvAPI_GPU_GetRamBusWidth_t	NvAPI_GPU_GetRamBusWidth_	= NULL;

bool GLimp_InitNvML() {
	nvmlReturn_t result;

	Com_Printf("" S_COLOR_YELLOW "...Initializing NVIDIA Management Library: ");

	result = nvmlInit();
	if (NVML_SUCCESS != result) {

		Com_Printf(S_COLOR_RED"nvmlInit: Failed!: %s\n", nvmlErrorString(result));
		return false;
	}
	else
		Com_Printf("" S_COLOR_GREEN "success.\n");

	char *version;
	version = malloc(NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE * sizeof(char *));
	result = nvmlSystemGetNVMLVersion(version, NVML_SYSTEM_NVML_VERSION_BUFFER_SIZE);
	if (NVML_SUCCESS != result) {

		Com_Printf(S_COLOR_RED"nvmlSystemGetNVMLVersion: Failed to query device count: %s\n", nvmlErrorString(result));
		return false;
	}
	Com_Printf("...NVML Version: " S_COLOR_GREEN "%s\n", version);

	result = nvmlDeviceGetCount(&nvmlPhysicalGpuCount);
	if (NVML_SUCCESS != result) {
		Com_Printf(S_COLOR_RED"nvmlDeviceGetCount: Failed to query device count: %s\n", nvmlErrorString(result));
		return false;
	}

	Com_Printf("\n==================================\n\n");
	return true;
}

void GLimp_InitNvApi() {

	NvAPI_Status ret = NVAPI_OK;
	NvAPI_ShortString ver, string;

	nvApiInit = false;
	gl_config.hdrDisplay = false;

	Com_Printf("\n==================================\n\n");

	Com_Printf("" S_COLOR_YELLOW "...Initializing NVIDIA API\n\n");

	// init nvapi
	ret = NvAPI_Initialize();

#ifdef _WIN64
	nv_hDLL = LoadLibrary("nvapi64.dll");
#else
	nv_hDLL = LoadLibrary("nvapi.dll");
#endif

	if (nv_hDLL) {
		NvAPI_GPU_QueryInterface	= (void*)GetProcAddress(nv_hDLL, "nvapi_QueryInterface");
		NvAPI_GPU_GetRamType		= NvAPI_GPU_QueryInterface(0x57F7CAAC);
		NvAPI_GPU_GetMemMarker		= NvAPI_GPU_QueryInterface(0x42AEA16A);
		NvAPI_GPU_GetRamBusWidth_	= NvAPI_GPU_QueryInterface(0x7975C581);
	}

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

	nvApiInit = true;
	NvApi_GetDisplayInfo();

	NvApi_SetUhdDisplays(r_useHdrDisplay->integer);

	Com_Printf("\n==================================\n\n");

	nvMlInit = false;
	if (GLimp_InitNvML())
		nvMlInit = true;
}

#define NV_UTIL_DOMAIN_GPU  0
#define NV_UTIL_DOMAIN_FB   1
#define NV_UTIL_DOMAIN_VID  2 //video decoder don't needed
#define NV_UTIL_DOMAIN_BUS  3

extern bool adlInit;

void R_GpuInfo_f(void) {

	NvAPI_Status		ret				= NVAPI_OK;
	NV_RAM_TYPE			memtype			= NV_RAM_TYPE_UNKNOWN;
	NV_RAM_MAKER		memmarker		= NV_RAM_MAKER_NONE;
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
	for (int i = 0; i < physicalGpuCount; i++) {

		Com_Printf("" S_COLOR_YELLOW ">" S_COLOR_WHITE "GPU" S_COLOR_GREEN "%i" S_COLOR_WHITE ":", i);
		ret = NvAPI_GPU_GetFullName(hPhysicalGpu[i], string);

		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetFullName() fail: %\n", string);
		}
		else
			Com_Printf(" " S_COLOR_GREEN "%s\n\n", string);

		ret = NvAPI_GPU_GetVbiosVersionString(hPhysicalGpu[i], string);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetVbiosVersionString() fail: %\n", string);
		}
		else
			Com_Printf("...Bios Version: " S_COLOR_GREEN "%s\n", string);

		int total = 0;
		float used = 0.0;
		NV_DISPLAY_DRIVER_MEMORY_INFO gpuMemoryStatus;
		gpuMemoryStatus.version = MAKE_NVAPI_VERSION(NV_DISPLAY_DRIVER_MEMORY_INFO_V2, 2);
		ret = NvAPI_GPU_GetMemoryInfo(hPhysicalGpu[i], (NV_DISPLAY_DRIVER_MEMORY_INFO *)&gpuMemoryStatus);
		if (NVAPI_OK != ret) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED "NvAPI_GPU_GetMemoryInfo() fail: %s\n", string);
		}
		else {
			used = (float)gpuMemoryStatus.dedicatedVideoMemory - (float)gpuMemoryStatus.curAvailableDedicatedVideoMemory;
			total = gpuMemoryStatus.dedicatedVideoMemory;

			total /= (1024 * 1024);
			used /= (1024 * 1024);
		}

		ret = NvAPI_GPU_GetRamType(hPhysicalGpu[i], &memtype);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetRamType() fail: %\n", string);
			Com_Printf("...Onboard Memory: " S_COLOR_GREEN " %i" S_COLOR_WHITE " Gb\n", total);
			Com_Printf("...Used GPU Memory: " S_COLOR_GREEN "%.2f" S_COLOR_WHITE " Gb\n", used);
		}
		else {
			Com_Printf("...Onboard Memory: " S_COLOR_GREEN " %i" S_COLOR_WHITE " Gb ", total);
			switch (memtype) {
			case NV_RAM_TYPE_SDRAM:
				Com_Printf(S_COLOR_GREEN"SDRAM ");
				break;
			case NV_RAM_TYPE_DDR1:
				Com_Printf(S_COLOR_GREEN"DDR1 ");
				break;
			case NV_RAM_TYPE_DDR2:
				Com_Printf(S_COLOR_GREEN"DDR2 ");
				break;
			case NV_RAM_TYPE_GDDR2:
				Com_Printf(S_COLOR_GREEN"GDDR2 ");
				break;
			case NV_RAM_TYPE_GDDR3:
				Com_Printf(S_COLOR_GREEN"GDDR3 ");
				break;
			case NV_RAM_TYPE_GDDR4:
				Com_Printf(S_COLOR_GREEN"GDDR4 ");
				break;
			case NV_RAM_TYPE_DDR3:
				Com_Printf(S_COLOR_GREEN"DDR3 ");
				break;
			case NV_RAM_TYPE_GDDR5:
				Com_Printf(S_COLOR_GREEN"GDDR5 ");
				break;
			case NV_RAM_TYPE_LPDDR2:
				Com_Printf(S_COLOR_GREEN"LPDDR2 ");
				break;
			case NV_RAM_TYPE_GDDR5X:
				Com_Printf(S_COLOR_GREEN"GDDR5X ");
				break;
			case NV_RAM_TYPE_GDDR6:
				Com_Printf(S_COLOR_GREEN"GDDR6 ");
				break;
			case NV_RAM_TYPE_GDDR6X:
				Com_Printf(S_COLOR_GREEN"GDDR6X ");
				break;
			case NV_RAM_TYPE_UNKNOWN:
			default:
				Com_Printf(S_COLOR_GREEN"Unknown %i ", memtype);
				break;
			}			
		}
		ret = NvAPI_GPU_GetMemMarker(hPhysicalGpu[i], &memmarker);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetMemMarker() fail: %\n", string);
		}
		else {
			switch (memmarker) {
			case NV_RAM_MAKER_SAMSUNG:
				Com_Printf("(Samsung)\n");
				break;
			case NV_RAM_MAKER_QIMONDA:
				Com_Printf("(Qimonda)\n");
				break;
			case NV_RAM_MAKER_ELPIDA:
				Com_Printf("(Elpida)\n");
				break;
			case NV_RAM_MAKER_ETRON:
				Com_Printf("(Etron)\n");
				break;
			case NV_RAM_MAKER_NANYA:
				Com_Printf("(Nanya)\n");
				break;
			case NV_RAM_MAKER_HYNIX:
				Com_Printf("(Hynix)\n");
				break;
			case NV_RAM_MAKER_MOSEL:
				Com_Printf("(Mosel)\n");
				break;
			case NV_RAM_MAKER_WINBOND:
				Com_Printf("(Winbond)\n");
				break;
			case NV_RAM_MAKER_ELITE:
				Com_Printf("(Elite)\n");
				break;
			case NV_RAM_MAKER_MICRON:
				Com_Printf("(Micron)\n");
				break;
			default:
				Com_Printf("(%i)\n", memmarker);
				break;
			}
			Com_Printf("...Used GPU Memory: " S_COLOR_GREEN "%.2f" S_COLOR_WHITE " Gb\n", used);
		}
		NvU32 busWidth = 0;
		ret = NvAPI_GPU_GetRamBusWidth_(hPhysicalGpu[i], &busWidth);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetRamBusWidth() fail: %\n", string);
		}
		else {
			Com_Printf("...Memory Bus Width: " S_COLOR_GREEN "%i" S_COLOR_WHITE " bit\n", busWidth);
		}

		NvU32 pWidth;
		uint pcieLines = 0;
		ret = NvAPI_GPU_GetCurrentPCIEDownstreamWidth(hPhysicalGpu[i], &pWidth);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetCurrentPCIEDownstreamWidth() fail: %\n", string);
		}
		else {
			pcieLines = (uint)pWidth;
			Com_Printf("...PCIe Lines: " S_COLOR_GREEN "%i" S_COLOR_WHITE "\n", pcieLines);
		}

		// get gpu temperature
		NV_GPU_THERMAL_SETTINGS	thermal;

		thermal.version = NV_GPU_THERMAL_SETTINGS_VER_2;
		thermal.count = NVAPI_MAX_THERMAL_SENSORS_PER_GPU;

		ret = NvAPI_GPU_GetThermalSettings(hPhysicalGpu[i], NVAPI_THERMAL_TARGET_ALL, &thermal);
		if (ret != NVAPI_OK) {
			NvAPI_GetErrorMessage(ret, string);
			Com_Printf(S_COLOR_RED"...NvAPI_GPU_GetThermalSettings() fail: %\n", string);
		}
		else {

			for (int j = 0; j < thermal.count; j++) {

				if (thermal.sensor[j].target == NVAPI_THERMAL_TARGET_GPU)
					Com_Printf("...GPU temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius (%s)\n", thermal.sensor[j].currentTemp, GLimp_NvApi_GetThermalController(thermal.sensor[i].controller));
				if (thermal.sensor[j].target == NVAPI_THERMAL_TARGET_MEMORY)
					Com_Printf("...VRAM temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius \n", thermal.sensor[j].currentTemp);
				if (thermal.sensor[j].target == NVAPI_THERMAL_TARGET_POWER_SUPPLY)
					Com_Printf("...Power Supply temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius \n", thermal.sensor[j].currentTemp);
				if (thermal.sensor[j].target == NVAPI_THERMAL_TARGET_BOARD)
					Com_Printf("...Board temperature: " S_COLOR_GREEN "%u" S_COLOR_WHITE " Celsius \n", thermal.sensor[j].currentTemp);
			}

		}

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

		if (nvMlInit) {
			nvmlDevice_t device;
			nvmlDeviceGetHandleByIndex(i, &device);
			
			uint32_t numFans, speedPercent;
			nvmlDeviceGetNumFans(device, &numFans);

			Com_Printf("\n...Found Fans: " S_COLOR_GREEN "%i\n", numFans);

			for (int f = 0; f < numFans; f++) {
				nvmlDeviceGetFanSpeed_v2(device, f, &speedPercent);
				Com_Printf(">Fan" S_COLOR_GREEN "%i" S_COLOR_WHITE ": " S_COLOR_GREEN "%i" S_COLOR_WHITE "%%\n", f, speedPercent);
			}
		}
		Com_Printf("\n==========================================================\n");
	}


}
