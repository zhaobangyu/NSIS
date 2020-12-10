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

#ifndef __STDUTILS_H__
#define __STDUTILS_H__

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <Shellapi.h>
#include <intrin.h>
#include "nsis/pluginapi.h"
#include "msvc_utils.h"

#define NSISFUNC(name) extern "C" void __declspec(dllexport) name(HWND hWndParent, int string_size, TCHAR* variables, stack_t** stacktop, extra_parameters* extra)

#define MAKESTR(VAR,LEN) \
	TCHAR *VAR = new TCHAR[LEN]; \
	memset((VAR), 0, sizeof(TCHAR) * (LEN))

#define DELETE_ARR(VAR,TYPE,LEN) do \
{ \
	if(VAR) \
	{ \
		memset((VAR), 0, sizeof(TYPE) * ((size_t)(LEN))); \
		delete [] (VAR); (VAR) = NULL; \
	} \
} \
while(0)

#define DELETE_STR(VAR,LEN) \
	DELETE_ARR(VAR, TCHAR, (((size_t)(LEN)) > 0U) ? ((size_t)(LEN)) : STRLEN(VAR))

#define REGSITER_CALLBACK() do \
{ \
	long _state;\
	do \
	{ \
		if((_state = _InterlockedCompareExchange(&g_bCallbackRegistred, -1, 0)) == 0) \
		{ \
			const int _ret = extra->RegisterPluginCallback(g_StdUtilsInstance, PluginCallback); \
			_InterlockedExchange(&g_bCallbackRegistred, (_state = ((_ret >= 0) ? 1 : 0))); \
		} \
	} \
	while(_state < 0); \
} \
while(0)

#define SHOW_VERBOSE(MSG) do \
{ \
	if(g_bStdUtilsVerbose) \
	{ \
		MessageBox(NULL, MSG, T("StdUtils"), MB_ICONERROR | MB_TASKMODAL); \
	} \
} \
while(0)

#endif //__STDUTILS_H__
