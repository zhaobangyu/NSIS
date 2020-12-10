// wstring.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)
// 08/13/06 (mv)

#include <windows.h>
#include <string.h>
#include "libct.h"

BEGIN_EXTERN_C

int _wcsicmp(const wchar_t *s1, const wchar_t *s2) {return wcsicmp(s1, s2);}
int wcsicmp(const wchar_t *s1, const wchar_t *s2)
{
    return lstrcmpiW(s1, s2);
}

int wcscmp(const wchar_t *s1, const wchar_t *s2)
{
	return lstrcmpW(s1, s2);
}

int wcsncmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	if (!n)
		return 0;

	for (size_t i = 0; i < n; i++)
	{
		if (!s1[i] || s1[i] != s2[i])
			return s1[i] - s2[i];
	}

	return 0;
}

int wcsnicmp(const wchar_t *s1, const wchar_t *s2, size_t n)
{
	return CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, s1, n, s2, n);
}

wchar_t *wcsdup(const wchar_t *src)
{
	if (!src)
		return 0;

	wchar_t *dst = (wchar_t*)malloc((wcslen(src)+1)*sizeof(wchar_t));
	wcscpy(dst, src);
	return dst;
}

wchar_t *_wcsdup(const wchar_t *src)
{
	return wcsdup(src);
}

wchar_t *wcscpy(wchar_t *dest, const wchar_t *src)
{
	return lstrcpyW(dest, src);
}

wchar_t *wcsncpy(wchar_t *dest, const wchar_t *src, size_t n)
{
	memcpy(dest, src, n*sizeof(wchar_t));
	size_t len = wcslen(src);
	if (n > len)
		memset(&dest[len], 0, (n-len)*sizeof(wchar_t));
	return dest;
}

size_t wcslen(const wchar_t *str)
{
	return lstrlenW(str);
}

const wchar_t *wcschr(const wchar_t *str, wchar_t ch)
{
	while (*str)
	{
		if (*str == ch)
			return str;
		str++;
	}
	return 0;
}

const wchar_t *wcsrchr(const wchar_t *str, wchar_t ch)
{
	const wchar_t *end = str + wcslen(str) + 1;
	while (end != str)
	{
		end--;
		if (*end == ch)
			return end;
	}
	return 0;
}

wchar_t *wcscat(wchar_t *dst, const wchar_t *src)
{
	return lstrcatW(dst, src);
}

const wchar_t *wcsstr(const wchar_t *str, const wchar_t *substr)
{
	int str_len = wcslen(str);
	int substr_len = wcslen(substr);

	if (substr_len == 0)
		return str;
	if (str_len < substr_len)
		return 0;
	for (int i = 0; i < (int)(str_len-substr_len+1); i++)
	{
		if (!wcscmp(&str[i], substr))
			return (const wchar_t *)(&str[i]);
	}
	return 0;
}

int mbtowc(wchar_t *pwc, register const char *s, size_t n)
{
	if (s == (const char *)NULL) return 0;
	if (n <= 0) return 0;
	if (pwc) *pwc = *s;
	return (*s != 0);
}

int wctomb(char *s, wchar_t wchar)
{
	if (!s) return 0;		/* no state dependent codings */

	*s = wchar;
	return 1;
}

size_t wcstombs(register char *s, register const wchar_t *pwcs, size_t n)
{
	register int i = n;

	while (--i >= 0) {
		if (!(*s++ = *pwcs++))
			break;
	}
	return n - i - 1;
}

size_t mbstowcs(register wchar_t *pwcs, register const char *s, size_t n)
{
	register int i = n;

	while (--i >= 0) {
		if (!(*pwcs++ = *s++))
			return n - i - 1;
	}
	return n - i;
}

END_EXTERN_C