// crt0tdll.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/13/06 (mv)

#include <windows.h>
#include "libct.h"

BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID);

EXTERN_C BOOL WINAPI _DllMainCRTStartup(HANDLE hInst, DWORD reason, LPVOID imp)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		_init_atexit();
		_init_file();
		_initterm(__xc_a, __xc_z);
	}

	BOOL ret = DllMain(hInst, reason, imp);

	if (reason == DLL_PROCESS_DETACH)
	{
		_doexit();
	}

	return ret;
}