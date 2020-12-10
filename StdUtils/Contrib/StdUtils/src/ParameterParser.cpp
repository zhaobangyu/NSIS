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

#pragma warning(disable:4996)

#include "Mutex.h"
#include "UnicodeSupport.h"
#include "CleanUp.h"

#include <malloc.h>
#include <msvc_utils.h>

//External
extern RTL_CRITICAL_SECTION g_pStdUtilsMutex;

//Stdlib
typedef struct { int newmode; } _startupinfo;
extern "C"
{
	int __getmainargs (int *_Argc, char    ***_Argv, char    ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
	int __wgetmainargs(int *_Argc, wchar_t ***_Argv, wchar_t ***_Env, int _DoWildCard, _startupinfo *_StartInfo);
}

//Unicode support
#ifdef UNICODE
#define GETMAINARGS __wgetmainargs
#else
#define GETMAINARGS __getmainargs
#endif

//Ugly Win2k hackage -> in Win2k __[w]getmainargs() did not have return value, so return value will be *undefined* on that OS!
#define IS_WIN2K (get_winver() < 0x501)

//Command-line parameters buffer
static volatile bool s_initialized = false;
static int s_argc = 0;
static TCHAR **s_argv = NULL, **s_envp = NULL;

///////////////////////////////////////////////////////////////////////////////
// INTERNAL FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

static inline DWORD get_winver(void)
{
	const DWORD dwVersion = GetVersion();
	return ((dwVersion & 0xFF) << 8) | ((dwVersion >> 8) & 0xFF);
}

static void free_mainargs(void)
{
	MutexLocker locker(&g_pStdUtilsMutex);
	if(s_initialized)
	{
		if(s_argv)
		{
			for (TCHAR **ptr = s_argv; (*ptr); ++ptr)
			{
				free(*ptr);
			}
			free(s_argv);
			s_argv = NULL;
			s_argc = 0;
		}
		s_initialized = false;
	}
}

static bool init_mainargs(void)
{
	MutexLocker locker(&g_pStdUtilsMutex);
	if(!s_initialized)
	{
		_startupinfo si = { 0 };
		if((GETMAINARGS(&s_argc, &s_argv, &s_envp, 0, &si) == 0) || IS_WIN2K)
		{
			if(s_argv != NULL)
			{
				s_initialized = true;
				cleanup_register_task(free_mainargs);
				return true;
			}
		}
		return false;
	}
	return true;
}

static bool valid_argname(const TCHAR *const arg_name)
{
	if((arg_name[0] == T('/')) || (arg_name[0] == T('\\')) || (arg_name[0] == T('-')))
	{
		return false;
	}
	for(size_t i = 0; arg_name[i]; i++)
	{
		if((!ISGRAPH(arg_name[i])) || (arg_name[i] == T('=')))
		{
			return false;
		}
	}
	return true;
}

static const TCHAR *get_argument_offset(const TCHAR *const argstr)
{
	if((argstr[0] == T('/')) && argstr[1])
	{
		return &argstr[1];
	}
	if((argstr[0] == T('-')) && (argstr[1] == T('-')) && argstr[2])
	{
		return &argstr[2];
	}
	return NULL;
}

static bool try_parse_arg(const TCHAR *const argstr, const TCHAR *const arg_name, TCHAR *const dest_buff, const size_t dest_size)
{
	const TCHAR *arg_offset = get_argument_offset(argstr);
	if(arg_offset && ISGRAPH(arg_offset[0]) && (arg_offset[0] != T('=')))
	{
		const TCHAR *separator = STRCHR(arg_offset, T('='));
		if(!(separator && separator[0]))
		{
			if(STRICMP(arg_offset, arg_name) == 0)
			{
				if(dest_buff && (dest_size > 0))
				{
					dest_buff[0] = T('\0');
				}
				return true;
			}
		}
		else
		{
			const size_t arg_len = STRLEN(arg_name);
			if((separator - arg_offset == arg_len) && (STRNICMP(arg_offset, arg_name, arg_len) == 0))
			{
				if(dest_buff && (dest_size > 0))
				{
					if(++separator)
					{
						STRNCPY(dest_buff, separator, dest_size);
						dest_buff[dest_size-1] = T('\0');
					}
					else
					{
						dest_buff[0] = T('\0');
					}
				}
				return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

bool commandline_get_arg(const TCHAR *const arg_name, TCHAR *const dest_buff, const size_t dest_size)
{
	if(arg_name && arg_name[0] && valid_argname(arg_name))
	{
		if(s_initialized || init_mainargs())
		{
			for(int i = 1; i < s_argc; i++)
			{
				if(try_parse_arg(STRTRIM_LEFT(s_argv[i]), arg_name, dest_buff, dest_size))
				{
					return true;
				}
			}
		}
	}
	return false;
}

int commandline_get_cnt(void)
{
	if(s_initialized || init_mainargs())
	{
		return (s_argc > 1) ? (s_argc - 1) : 0;
	}
	return -1;
}

bool commandline_get_raw(const int index, TCHAR *const dest_buff, const size_t dest_size)
{
	if(index >= 0)
	{
		if(s_initialized || init_mainargs())
		{
			const int actual_index = index + 1;
			if(actual_index < s_argc)
			{
				STRNCPY(dest_buff, s_argv[actual_index], dest_size);
				dest_buff[dest_size-1] = T('\0');
				return true;
			}
		}
	}
	return false;
}

const TCHAR *commandline_get_all(void)
{
	const TCHAR *cmd = GetCommandLine();
	if((!cmd) || (!cmd[0]))
	{
		static const TCHAR *error = T("error");
		return error;
	}

	size_t pos = 0;
	while(WHITESPACE(cmd[pos])) pos++;

	bool flag = false;
	while(cmd[pos])
	{
		if(cmd[pos] == T('"'))
		{
			flag = (!flag);
		}
		else
		{
			if((!flag) && WHITESPACE(cmd[pos]))
			{
				break;
			}
		}
		pos++;
	}

	while(WHITESPACE(cmd[pos])) pos++;
	return &cmd[pos];
}
