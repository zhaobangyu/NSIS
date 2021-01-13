// struplwr.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include <string.h>
#include "libct.h"

BEGIN_EXTERN_C

char *strupr(char *s)
{
    CharUpperBuffA(s, lstrlenA(s));
    return s;
}

wchar_t *wcsupr(wchar_t *s)
{
    CharUpperBuffW(s, lstrlenW(s));
    return s;
}

char *strlwr(char *s)
{
    CharLowerBuffA(s, lstrlenA(s));
    return s;
}

wchar_t *wcslwr(wchar_t *s)
{
    CharLowerBuffW(s, lstrlenW(s));
    return s;
}

int toupper(int c)
{
	if (c < 'a' || c > 'z')
		return c;
	return c-0x20;
}

wint_t towupper(wint_t c)
{
	// yes, that's right:  no &c - see the docs
	return (wint_t)CharUpperW((LPWSTR)c);
}

int tolower(int c)
{
	if (c < 'A' || c > 'Z')
		return c;
	return c+0x20;
}

wint_t towlower(wint_t c)
{
	// yes, that's right:  no &c - see the docs
	return (wint_t)CharLowerW((LPWSTR)c);
}

END_EXTERN_C