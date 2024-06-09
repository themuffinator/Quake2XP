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
{0x056A, "Wacom Co., Ltd"},
{0x0955, "NVIDIA Corp."},
{0x28de, "Valve Corp."},
{0x0738, "Mad Catz, Inc."},
{0x0DB0, "Micro-Star International Co., Ltd"},
{0x1C4F, "Beijing Sigmachip Co., Ltd."},
{0x0458, "KYE Systems Corp."},
};

#define NUM_VENDORS ( sizeof( usb_Vendors ) / sizeof( usb_Vendors[0] ) )

typedef struct product_s {
	DWORD Id;
	const char* description;
} product_t;


product_t product[] = {
//MSI
{0x0D08, "Clutch GM08 Gaming Mouse."},
//SteelSeries
{0x1361,	"Ideazon Sensei."},
{0x1720,	"Mouse."},
{0x1729,	"Rival 110 Gaming Mouse."},
//Mad Catz
{0x1703, "R.A.T.3 Mouse."},
{0xCB03, "Saitek P3200 Rumble Pad."},
{0x4726, "Xbox 360 Controller."},
{0x4716, "Wired Xbox 360 Controller."},
//---------a4tech
{0x0006, "Optical Mouse WOP - 35 / Trust 450L Optical Mouse."},
{0x000A, "Optical Mouse Opto 510D / OP - 620D."},
{0x000E, "X-F710F Optical Mouse 3xFire Gaming Mouse."},
{0x002A, "Wireless Optical Mouse NB - 30."},
{0x022B, "Wireless Mouse(Battery Free)."},
{0x024F, "RF Receiverand G6 - 20D Wireless Optical Mouse."},
{0x032B, "Wireless Mouse(Battery Free)."},
{0x09DA, "Bloody V8 Mouse."},
{0x1068, "Bloody A90 Mouse."},
{0x112C, "Bloody V5 Mous.e"},
{0x3A60, "Bloody V8M Core 2 Mouse."},
{0x8090, "X-718BK Oscar Optical Gaming Mouse."},
{0x9033, "X-718BK Optical Mouse."},
{0x9066, "F3 V-Track Gaming Mouse."},
{0x9090, "XL-730K/XL-740K/XL-750BK/XL-755BK Mice."},
{0xF613, "Bloody V7M Mouse."},
{0xFA1C, "Bloody B135N Keyboard"},
{0xE5C0, "Bloody Keyboard"},

	// --------logitech
{0XC70E,	"MX1000 Bluetooth Laser Mouse."},
{0XC518,	"MX610 Laser Cordless Mous.e"},
{0XC51A,	"MX Revolution / G7 Cordless Mous.e"},
{0XC51B,	"V220 Cordless Optical Mouse for Notebooks."},
{0XC510,	"Cordless Mouse."},
{0XC514,	"Cordless Mouse."},
{0XC50A,	"Cordless Mouse."},
{0XC402,	"Marble Mouse(2 - button)."},
{0XC408,	"Marble Mouse(4 - button)."},
{0XC247,	"G100S Optical Gaming Mouse."},
{0XC24A,	"G600 Gaming Mouse."},
{0XC24C,	"G400s Optical Mouse."},
{0XC24E,	"G500s Laser Gaming Mouse."},
{0XC245,	"G400 Optical Mouse."},
{0XC246,	"Gaming Mouse G300."},
{0XC077,	"M105 Optical Mouse."},
{0XC07C,	"G700s Rechargeable Gaming Mouse."},
{0XC07D,	"G502 Mouse."},
{0XC07E,	"G402 Gaming Mouse."},
{0XC080,	"G303 Gaming Mouse."},
{0XC083,	"G403 Prodigy Gaming Mouse."},
{0XC084,	"G203 Gaming Mouse."},
{0XC063,	"DELL Laser Mouse."},
{0XC064,	"M110 Corded optical mouse."},
{0XC066,	"G9x Laser Mouse."},
{0XC068,	"G500 Laser Mouse."},
{0xC069,	"M500 Corded Mouse."},
{0xC092,	"G102 LightSync Mouse."},

{0xc21d,	"Gamepad F310."},	
{0xc21e,	"Gamepad F510."},
{0xc21f,	"Gamepad F710."},
{0xc242,	"Chillstream Controller."},
{0xc20a,	"WingMan RumblePad."},
{0xc208,	"WingMan Gamepad Extreme."},
{0xc209,	"WingMan Gamepad."},
{0xc20b,	"WingMan Action Pad."},
{0xc20c,	"WingMan Precision."},

//------------ microsoft
{0X0202,	"Xbox Controller."},
{0X0285,	"Xbox Controller S."},
{0X0289,	"Xbox Controller S."},

{0X028E,	"Xbox360 Controller."},
{0X028F,	"Xbox360 Wireless Controller."},

{0xb400,	"NVIDIA Shield streaming controller"}, //xbox360 type
{0x7210,	"Nvidia Shield local controller."},

{0x02d1,	"Xbox One Controller." },
{0x02dd,	"Xbox One Controller (Firmware 2015)." },
{0x02e0,	"Xbox One S Controller (Bluetooth)." },
{0x02e3,	"Xbox One Elite Controller." },
{0x02ea,	"Xbox One S Controller." },
{0x02fd,	"Xbox One S Controller (Bluetooth)." },
{0x02ff,	"Xbox One S Controller (Usb)." }, // valve say its Microsoft X-Box One Elite pad, lol
{0x0b00,	"Xbox One Elite 2 Controller." },
{0x0b02,	"Xbox One Elite 2 Controller (Bluetooth)." },
{0x0b05,	"Xbox One Elite 2 Controller (Bluetooth)." },
{0x0b0a,	"Xbox Adaptive Controller." },
{0x0b0c,	"Xbox Adaptive Controller (Bluetooth)." },
{0x0b12,	"Xbox Series X Controller." },
{0x0b13,	"Xbox Series X Controller (BLE)." },
{0x0b20,	"Xbox One S Controller (BLE)." },
{0x0b21,	"Xbox Adaptive Controller (BLE)." },
{0x0b22,	"Xbox One Elite 2 Controller (BLE)." },
//---------- sony
{0x0268,	"PS3 Controller." },
{0x05c4,	"PS4 Controller." },
{0x05c5,	"STRIKEPAD PS4 Grip Add-on."},
{0x09cc,	"PS4 Slim Controller."},
{0x0ce6,	"PS5 Slim Controller." },

//----------- razer ps4
{0x1000,	"Raiju PS4 Controller."},	
{0X0401,	"Panthera PS4 Controller."},	
{0x1007,	"Raiju 2 Tournament edition."},	
{0x100A,	"Raiju 2 Tournament edition (Bluetooth)."},
{0x1004,	"Razer Raiju 2 Ultimate"},
{0x1009,	"Raiju 2 Ultimate (Bluetooth)."},
//----------- razer xbox360
{0x0037,	"Sabertooth." },
{0xfd00,	"Onza Tournament Edition"},
{0xfd01,	"Onza Classic Edition"},
{0xfe00,	"Sabertooth"},
//----------- razer xbox one
{0x0a03,	"Wildcat." },

// Valve Steam Controller
{0x1101,	"Legacy Steam Controller (CHELL)"},
{0x1102,	"Wired Steam Controller (D0G)"},
{0x1105,	"Bluetooth Steam Controller (D0G)"},
{0x1106,	"Bluetooth Steam Controller (D0G)"},
{0x1142,	"Wireless Steam Controller"},
{0x1201,	"Wired Steam Controller (HEADCRAB)"},
{0x1202,	"Bluetooth Steam Controller (HEADCRAB)"},

{0x0026,	"Beijing Keyboard"},
{0x0186,	"Genius DX-120 Mouse"},
};

#define NUM_INPUT_DEVICES ( sizeof( product ) / sizeof( product[0] ) )
