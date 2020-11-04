/*
* This is an open source non-commercial project. Dear PVS-Studio, please check it.
* PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
*/

typedef struct usbVendorId_s {
	DWORD vendorId;
	const char *description;
} usbVendorId_t;

usbVendorId_t usb_Vendors[] = {
{0x03F0, "HP, Inc"},
{0x045E, "Microsoft Corp."},
{0x046A, "Cherry GmbH."},
{0x046D, "Logitech Inc."},
{0x04A5, "Acer Peripherals Inc. (now BenQ Corp.)"},
{0x04B3, "IBM Corp."},
{0x04F3, "Elan Microelectronics Corp."},
{0x050D, "Belkin Components"},
{0x054C, "Sony Corp."},
{0x055D, "Samsung Electro - Mechanics Co."},
{0x05AC, "Apple Inc."},
{0x06A3, "Saitek PLC"},
{0x09DA, "A4Tech Co., Ltd."},
{0x1038, "SteelSeries"},
{0x1241, "Belkin"},
{0x1267, "Logic3 (LG)"},
{0x1532, "Razer USA, Ltd"},
{0x1689, "Razer USA, Ltd"},
{0x17EF, "Lenovo"},
{0x1D57, "Xenta"},
{0x2516, "Cooler Master Co., Ltd."},
{0x413C, "Dell Computer Corp."},
{0x056A, "Wacom Co., Ltd"}
};

#define NUM_VENDORS ( sizeof( usb_Vendors ) / sizeof( usb_Vendors[0] ) )
/*
typedef struct Logitech_s {
	DWORD Id;
	const char* description;
} Logitech_t;

Logitech_t logitech_mice[] = {
{0XC70E,	"MX1000 Bluetooth Laser Mouse"},
{0XC518,	"MX610 Laser Cordless Mouse"},
{0XC51A,	"MX Revolution / G7 Cordless Mouse"},
{0XC51B,	"V220 Cordless Optical Mouse for Notebooks"},
{0XC510,	"Cordless Mouse"},
{0XC514,	"Cordless Mouse"},
{0XC50A,	"Cordless Mouse"},
{0XC402,	"Marble Mouse(2 - button)"},
{0XC408,	"Marble Mouse(4 - button)"},
{0XC247,	"G100S Optical Gaming Mouse"},
{0XC24A,	"G600 Gaming Mouse"},
{0XC24C,	"G400s Optical Mouse"},
{0XC24E,	"G500s Laser Gaming Mouse"},
{0XC245,	"G400 Optical Mouse"},
{0XC246,	"Gaming Mouse G300"},
{0XC077,	"M105 Optical Mouse"},
{0XC07C,	"G700s Rechargeable Gaming Mouse"},
{0XC07D,	"G502 Mouse"},
{0XC07E,	"G402 Gaming Mouse"},
{0XC080,	"G303 Gaming Mouse"},
{0XC083,	"G403 Prodigy Gaming Mouse"},
{0XC084,	"G203 Gaming Mouse"},
{0XC063,	"DELL Laser Mouse"},
{0XC064,	"M110 corded optical mouse"},
{0XC066,	"G9x Laser Mouse"},
{0XC068,	"G500 Laser Mouse"},

};
*/