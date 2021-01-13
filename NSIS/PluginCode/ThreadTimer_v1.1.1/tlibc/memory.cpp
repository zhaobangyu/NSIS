// memory.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/13/06 (mv)

#include <windows.h>
#include <string.h>
#include "libct.h"

BEGIN_EXTERN_C

int memcmp(const void *b1, const void *b2, size_t n)
{
	const unsigned char *p1 = (const unsigned char*)b1;
	const unsigned char *p2 = (const unsigned char*)b2;

	if (!n)
		return 0;
	for (size_t i = 0; i < n; i++)
	{
		if (p1[i] != p2[i])
			return p1[i]-p2[i];
	}
	return 0;
}

void *memset(void *dst, int val, size_t size)
{
	char *realdst = (char*)dst;
	for (size_t i = 0; i < size; i++)
		realdst[i] = (char)val;
	return dst;
}

void *memcpy(void *dst, const void *src, size_t size)
{
	char *_dst = (char*)dst;
	const char *_src = (char*)src;
	for (size_t i = 0; i < size; i++)
		_dst[i] = _src[i];
	return dst;
}

void *memmove(void *dst, const void *src, size_t count)
{
	void *ret = dst;
	if (dst <= src || (char *)dst >= ((char *)src + count))
	{
		// Non-Overlapping Buffers
		// copy from lower addresses to higher addresses
		while (count--)
		{
			*(char*)dst = *(char*)src;
			dst = (char*)dst + 1;
			src = (char*)src + 1;
		}
	}
	else
	{
		// Overlapping Buffers
		// copy from higher addresses to lower addresses
		dst = (char*)dst + count - 1;
		src = (char*)src + count - 1;

		while (count--)
		{
			*(char*)dst = *(char*)src;
			dst = (char*)dst - 1;
			src = (char*)src - 1;
		}
	}

	return ret;
}

END_EXTERN_C