// string.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include <string.h>
#include <tchar.h>
#include "libct.h"

BEGIN_EXTERN_C

int strcmpi(const char *s1, const char *s2)
{
    return lstrcmpiA(s1, s2);
}

int stricmp(const char *s1, const char *s2) {return _stricmp(s1, s2);}
int _stricmp(const char *s1, const char *s2)
{
    return lstrcmpiA(s1, s2);
}

int strcmp(const char *s1, const char *s2)
{
	return lstrcmpA(s1, s2);
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	if (!n)
		return 0;

	const unsigned char *p1 = (const unsigned char*)s1;
	const unsigned char *p2 = (const unsigned char*)s2;

	for (size_t i = 0; i < n; i++)
	{
		if (!p1[i] || p1[i] != p2[i])
			return p1[i] - p2[i];
	}

	return 0;
}

int strnicmp(const char *s1, const char *s2, size_t n)
{
	return CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, s1, n, s2, n);
}

char *strdup(const char *src)
{
	if (!src)
		return 0;

	char *dst = (char*)malloc(strlen(src)+1);
	strcpy(dst, src);
	return dst;
}

char *_strdup(const char *src)
{
	return strdup(src);
}

char *strcpy(char *dest, const char *src)
{
	return lstrcpyA(dest, src);
}

char *strncpy(char *dest, const char *src, size_t n)
{
	memcpy(dest, src, n);
	size_t len = strlen(src);
	if (n > len)
		memset(&dest[len], 0, n-len);
	return dest;
}

size_t strlen(const char *str)
{
	return lstrlenA(str);
}

const char *strchr(const char *str, int ch)
{
	while (*str)
	{
		if (*str == ch)
			return str;
		str++;
	}
	return 0;
}

const char *strrchr(const char *str, int ch)
{
	const char *end = str + strlen(str) + 1;
	while (end != str)
	{
		end--;
		if (*end == ch)
			return end;
	}
	return 0;
}

char *strcat(char *dst, const char *src)
{
	return lstrcatA(dst, src);
}

const char *strstr(const char *str, const char *substr)
{
	int str_len = strlen(str);
	int substr_len = strlen(substr);

	if (substr_len == 0)
		return str;
	if (str_len < substr_len)
		return 0;
	for (int i = 0; i < (int)(str_len-substr_len+1); i++)
	{
		if (!strcmp(&str[i], substr))
			return (const char *)(&str[i]);
	}
	return 0;
}

END_EXTERN_C