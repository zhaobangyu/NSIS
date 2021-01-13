///////////////////////////////////////////////////////////////////////////////
// StdUtils plug-in for NSIS
// Copyright (C) 2004-2018 LoRd_MuldeR <MuldeR2@GMX.de>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// http://www.gnu.org/licenses/lgpl-2.1.txt
///////////////////////////////////////////////////////////////////////////////

#ifndef __MSVC_FUNC_H__
#define __MSVC_FUNC_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RAND_MAX 0x7fff
#define INT_MAX 2147483647

#ifndef _INC_STDLIB
long __declspec(dllimport) time(long *time);
long __declspec(dllimport) clock(void);
long __declspec(dllimport) _getpid(void);
int  __declspec(dllimport) rand(void);
void __declspec(dllimport) srand(unsigned int _Seed);
int  __declspec(dllimport) abs(_In_ int _X);
void __declspec(dllimport) _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);
void __declspec(dllimport) _wsplitpath(const wchar_t *path, wchar_t *drive, wchar_t *dir, wchar_t *fname, wchar_t *ext);
#endif //_INC_STDLIB

int     __declspec(dllimport) _snwprintf(wchar_t *buffer, size_t count, const wchar_t *format, ...);
int     __declspec(dllimport) _snprintf(char *buffer, size_t count, const char *format, ...);
int     __declspec(dllimport) sscanf(const char *input, const char * format, ...);
int     __declspec(dllimport) swscanf(const wchar_t *input, const wchar_t * format, ...);
char    __declspec(dllimport) *setlocale(int category, const char *locale);
wchar_t __declspec(dllimport) *_wsetlocale(int category, const wchar_t *locale);

uintptr_t __declspec(dllimport) _beginthreadex( 
	void *security,
	unsigned stack_size,
	unsigned (__stdcall *start_address)(void*),
	void *arglist,
	unsigned initflag,
	unsigned *thrdaddr
);

#ifdef __cplusplus
}
#endif

#endif //__MSVC_FUNC_H__
