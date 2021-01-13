// printf.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "libct.h"

// Force the linker to include USER32.LIB
#pragma comment(linker, "/defaultlib:user32.lib")

BEGIN_EXTERN_C

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int ret = vprintf(format, args);
    va_end(args);

	return ret;
}

int wprintf(const wchar_t *format, ...)
{
	va_list args;
	va_start(args, format);
	int ret = vwprintf(format, args);
	va_end(args);

	return ret;
}



int vprintf(const char *format, va_list args)
{
	char szBuff[1024];

	int retValue = wvsprintfA(szBuff, format, args);

	fwrite(szBuff, retValue, 1, stdout);

	return retValue;
}

int vwprintf(const wchar_t *format, va_list args)
{
	wchar_t buf[1024];
	int ret = wvsprintfW(buf, format, args);

	char ansibuf[1024];
	WideCharToMultiByte(CP_ACP, 0, buf, -1, ansibuf, sizeof(ansibuf), 0, 0);
	fwrite(ansibuf, ret, 1, stdout);

	return ret;
}

END_EXTERN_C