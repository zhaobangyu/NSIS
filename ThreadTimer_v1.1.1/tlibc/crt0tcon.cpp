// crt0tcon.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include "libct.h"
#include <stdio.h>

#pragma comment(linker, "/nodefaultlib:libc.lib")
#pragma comment(linker, "/nodefaultlib:libcmt.lib")
#pragma comment(linker, "/nodefaultlib:libcmtd.lib")

EXTERN_C int _tmain(int, TCHAR **, TCHAR **);    // In user's code

#ifdef UNICODE
EXTERN_C void wmainCRTStartup()
#else
EXTERN_C void mainCRTStartup()
#endif
{
    int argc = _init_args();
    _init_atexit();
	_init_file();
    _initterm(__xc_a, __xc_z);

    int ret = _tmain(argc, _argv, 0);

	_doexit();
	_term_args();
    ExitProcess(ret);
}