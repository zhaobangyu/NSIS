// sprintf.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include <stdio.h>
#include "libct.h"

#define EOF     (-1)

// Force the linker to include USER32.LIB
#pragma comment(linker, "/defaultlib:user32.lib")

EXTERN_C int sprintf(char *buffer, const char *format, ...)
{
    int retValue;
    va_list argptr;
          
    va_start(argptr, format);
    retValue = vsprintf(buffer, format, argptr);
    va_end(argptr);

    return retValue;
}

EXTERN_C int _snprintf(char *dest, size_t n, const char *fmt, ...)
{
	n;

	va_list args;
	va_start(args, fmt);
	int retValue = vsprintf(dest, fmt, args);
	va_end(args);
	return retValue;
}

EXTERN_C int vsnprintf(char *dest, size_t n, const char *fmt, va_list args)
{
	n;
	return vsprintf(dest, fmt, args);
}