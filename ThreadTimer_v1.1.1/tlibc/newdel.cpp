// newdel.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>

void *operator new(unsigned int s)
{
    return HeapAlloc(GetProcessHeap(), 0, s);
}

void operator delete(void *p)
{
    HeapFree(GetProcessHeap(), 0, p);
}

void operator delete[](void *p)
{
	HeapFree(GetProcessHeap(), 0, p);
}

void *operator new[](unsigned int s)
{
	return HeapAlloc(GetProcessHeap(), 0, s);
}