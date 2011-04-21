/*=========================== 
Quake2xp c++ function wrapper
===========================*/


#include "ddraw.h"
#pragma comment(lib, "ddraw")
#pragma comment(lib, "dxguid")

#if 0
int getAvailableVideoMemory_win32_1()
{
	HRESULT			hr			=	DD_OK;
	LPDIRECTDRAW	ddraw		=	NULL;
	LPDIRECTDRAW7	ddraw7		=	NULL;	
	DDCAPS			driver_caps	=	{0};
	DDCAPS			hel_caps	=	{0};
	
	
	driver_caps.dwSize=sizeof(DDCAPS);
	hel_caps.dwSize=sizeof(DDCAPS);

	hr = DirectDrawCreate((GUID*)DDCREATE_HARDWAREONLY, &ddraw, NULL);// на проверки забил
	hr = ddraw->QueryInterface(IID_IDirectDraw7,(LPVOID*)&ddraw7);
	hr = ddraw7->GetCaps(&driver_caps, &hel_caps);

	ddraw7->Release();
	ddraw->Release();

	return driver_caps.dwVidMemTotal >> 20;
}
#endif



int getAvailableLocalVideoMemory(){

DWORD localVidMem = NULL;
IDirectDraw7* pDirectDraw;
HRESULT hr = CoCreateInstance(CLSID_DirectDraw7, NULL, CLSCTX_ALL, 
                              IID_IDirectDraw7, (void**) &pDirectDraw);


if(!FAILED(hr))
{
  hr = IDirectDraw7_Initialize(pDirectDraw, NULL);
  if (!FAILED(hr))
  {
    
	DDSCAPS2 QueryCaps = { 0 };
    DWORD dwTotal, dwFree;

    QueryCaps.dwCaps = DDSCAPS_LOCALVIDMEM;
    hr = pDirectDraw->GetAvailableVidMem(&QueryCaps, &dwTotal, &dwFree);
    if (!FAILED(hr))
      localVidMem = dwTotal;  	 
  }
	
  pDirectDraw->Release();
}

return localVidMem >> 20;
}


int getAvailableTotalVideoMemory(){

DWORD totalVidMem = NULL;
IDirectDraw7* pDirectDraw;
HRESULT hr = CoCreateInstance(CLSID_DirectDraw7, NULL, CLSCTX_ALL, 
                              IID_IDirectDraw7, (void**) &pDirectDraw);

if(!FAILED(hr))
{
  hr = IDirectDraw7_Initialize(pDirectDraw, NULL);
  if (!FAILED(hr))
  {
    
	DDSCAPS2 QueryCaps = { 0 };
    DWORD dwTotal, dwFree;


    QueryCaps.dwCaps = DDSCAPS_TEXTURE;
    hr = pDirectDraw->GetAvailableVidMem(&QueryCaps, &dwTotal, &dwFree);
    if (!FAILED(hr))
      totalVidMem = dwTotal;  	 
  }
	
  pDirectDraw->Release();
}

return totalVidMem >> 20;
}



extern "C"{

	int __cdecl cpp_getAvailableLocalVideoMemory(){
	
	return getAvailableLocalVideoMemory();
	}
	
	int __cdecl cpp_getAvailableTotalVideoMemory(){
	
	return getAvailableTotalVideoMemory();
	}

}
