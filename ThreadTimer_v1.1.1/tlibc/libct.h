// libct.h
// http://www.codeproject.com/KB/library/tlibc.aspx

#pragma once

#include <tchar.h>

#ifdef __cplusplus
#define EXTERN_C			extern "C"
#define BEGIN_EXTERN_C		extern "C" {
#define END_EXTERN_C		}
#else
#define EXTERN_C
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif



BEGIN_EXTERN_C

extern TCHAR *_argv[];
int _init_args();
void _term_args();

typedef void (__cdecl *_PVFV)();
extern _PVFV __xc_a[], __xc_z[];    /* C++ initializers */

void _initterm(_PVFV *pfbegin, _PVFV *pfend);
void _init_atexit();
void _doexit();

END_EXTERN_C

void _init_file();