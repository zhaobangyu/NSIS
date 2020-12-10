// puts.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include <stdio.h>
#include "libct.h"

EXTERN_C int puts(const char *s)
{
    //DWORD cbWritten;
    //HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	int bw = fwrite(s, lstrlenA(s), 1, stdout);
	bw += fwrite("\n", 1, 1, stdout);
	return bw;

    //WriteFile(hStdOut, s, lstrlenA(s)*sizeof(char), &cbWritten, 0);
    //WriteFile(hStdOut, "\r\n", 2, &cbWritten, 0);

    //return (int)(cbWritten ? cbWritten : EOF);
}

EXTERN_C int _putws(const wchar_t *s)
{
	//DWORD bw;
	//HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// _putws converts unicode to ascii before writing to stdout!
	char bfr[1024];
	WideCharToMultiByte(CP_ACP, 0, s, -1, bfr, sizeof(bfr), 0, 0);
	return puts(bfr);

	//WriteFile(hStdOut, s, lstrlenW(s)*sizeof(wchar_t), &bw, 0);
	//WriteFile(hStdOut, L"\r\n", 2*sizeof(wchar_t), &bw, 0);

	//return (int)(bw ? bw : EOF);
}

