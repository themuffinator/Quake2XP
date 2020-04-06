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
{0x17EF, "Lenovo"},
{0x1D57, "Xenta"},
{0x2516, "Cooler Master Co., Ltd."},
{0x413C, "Dell Computer Corp."},
{0x056A, "Wacom Co., Ltd"}
};

#define NUM_VENDORS ( sizeof( usb_Vendors ) / sizeof( usb_Vendors[0] ) )