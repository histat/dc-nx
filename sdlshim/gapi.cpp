
#include "shim.h"
#include "gapi.fdh"

#define GX_FULLSCREEN	 0x01
int (*GXOpenDisplay)(HWND hWnd, DWORD dwFlags);
int (*GXCloseDisplay)();
void *(*GXBeginDraw)();
int (*GXEndDraw)();
void (*GXOpenInput)();
void (*GXCloseInput)();

uint16_t *vram;
uint16_t *saved_vram;


bool gapi_init(void)
{
	LoadFunctions();
	
	if (!GXOpenDisplay(NULL, GX_FULLSCREEN))
	{
		MSGBOX("GXOpenDisplay failed!");
		return 1;
	}
	
	vram = (uint16_t *)GXBeginDraw();
	if (!vram)
	{
		MSGBOX("GXBeginDraw failed!");
		return 1;
	}
	
	saved_vram = (uint16_t *)malloc(SCREEN_PITCH * SCREEN_HEIGHT);
	memcpy(saved_vram, vram, SCREEN_PITCH * SCREEN_HEIGHT);
	memset(vram, 0, SCREEN_PITCH * SCREEN_HEIGHT);
	
	GXOpenInput();
	return 0;
}

void gapi_close()
{
	if (saved_vram)
	{
		memcpy(vram, saved_vram, SCREEN_PITCH * SCREEN_HEIGHT);
		free(saved_vram);
	}
	
	GXEndDraw();
	GXCloseInput();
	GXCloseDisplay();
}


int gapi_GetScreenSize(int *w, int *h)
{
	*w = 240;
	*h = 240;
	return 0;
}

/*
void c------------------------------() {}
*/

static void LoadFunctions(void)
{
	GXOpenDisplay = (int(*)(HWND__*, DWORD))LoadFunc(L"gx", L"?GXOpenDisplay@@YAHPAUHWND__@@K@Z");
	GXCloseDisplay = (int(*)())LoadFunc(L"gx", L"?GXCloseDisplay@@YAHXZ");
	GXBeginDraw = (void *(*)())LoadFunc(L"gx", L"?GXBeginDraw@@YAPAXXZ");
	GXEndDraw = (int(*)())LoadFunc(L"gx", L"?GXEndDraw@@YAHXZ");
	GXOpenInput = (void(*)())LoadFunc(L"gx", L"?GXOpenInput@@YAHXZ");
	GXCloseInput = (void(*)())LoadFunc(L"gx", L"?GXCloseInput@@YAHXZ");
}

static void *LoadFunc(const wchar_t *dll, const wchar_t *funcname)
{
    HINSTANCE hi = LoadLibrary(dll);
	if (!hi)
	{
		MSGBOX("Unable to load library.");
		exit(1);
    }

    void *func = (void*)GetProcAddress(hi, funcname);
    if (!func)
    {
		MSGBOX("Unable to find function in library.");
		exit(1);
    }

    return func;
}
