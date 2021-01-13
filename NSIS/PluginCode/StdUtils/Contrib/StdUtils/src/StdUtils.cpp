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

#include "StdUtils.h"
#include "ShellExecAsUser.h"
#include "ParameterParser.h"
#include "InvokeShellVerb.h"
#include "UnicodeSupport.h"
#include "DetectOsVersion.h"
#include "WinUtils.h"
#include "FileUtils.h"
#include "HashUtils.h"
#include "TimerUtils.h"
#include "PathUtils.h"
#include "Base64.h"
#include "ProtectedData.h"
#include "CleanUp.h"

//External
bool g_bStdUtilsVerbose = false;
RTL_CRITICAL_SECTION g_pStdUtilsMutex;
HINSTANCE g_StdUtilsInstance = NULL;

//Global
static volatile long g_bCallbackRegistred = 0L;

///////////////////////////////////////////////////////////////////////////////
// DLL MAIN
///////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		InitializeCriticalSection(&g_pStdUtilsMutex);
		g_bCallbackRegistred = 0;
		g_bStdUtilsVerbose = false;
		g_StdUtilsInstance = hinstDLL;
	}
	else if(fdwReason == DLL_PROCESS_DETACH)
	{
		cleanup_execute_tasks();
		DeleteCriticalSection(&g_pStdUtilsMutex);
	}
	return TRUE;
}

static UINT_PTR PluginCallback(enum NSPIM msg)
{
	switch(msg)
	{
	case NSPIM_UNLOAD:
	case NSPIM_GUIUNLOAD:
		break;
	default:
		OutputDebugStringA("StdUtils: Unknown callback message. Take care!\n");
		break;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// TIME UTILS
///////////////////////////////////////////////////////////////////////////////

static const unsigned __int64 FTIME_SECOND = 10000000ui64;
static const unsigned __int64 FTIME_MINUTE = 60ui64 * FTIME_SECOND;
static const unsigned __int64 FTIME_HOUR   = 60ui64 * FTIME_MINUTE;
static const unsigned __int64 FTIME_DAY    = 24ui64 * FTIME_HOUR;

static unsigned __int64 getFileTime(void)
{
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	
	FILETIME filetime;
	if(!SystemTimeToFileTime(&systime, &filetime))
	{
		return 0;
	}

	ULARGE_INTEGER uli;
	uli.LowPart = filetime.dwLowDateTime;
	uli.HighPart = filetime.dwHighDateTime;

	return uli.QuadPart;
}

NSISFUNC(Time)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const long t = time(NULL);
	pushint(t);
}

NSISFUNC(GetMinutes)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const unsigned __int64 ftime = getFileTime() / FTIME_MINUTE;
	pushint(static_cast<int>(ftime));
}

NSISFUNC(GetHours)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const unsigned __int64 ftime = getFileTime() / FTIME_HOUR;
	pushint(static_cast<int>(ftime));
}

NSISFUNC(GetDays)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const unsigned __int64 ftime = getFileTime() / FTIME_DAY;
	pushint(static_cast<int>(ftime));
}

///////////////////////////////////////////////////////////////////////////////
// PRNG FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

#include "RandUtils.h"

static int rand_in_range(const int min = 0, const int max = INT_MAX)
{
	if(min < max)
	{
		const unsigned int range = static_cast<unsigned int>(max - min) + 1U;
		if(range)
		{
			const unsigned int nbins = ((UINT_MAX - (range - 1U)) / range) + 1U;
			const unsigned int limit = nbins * range;
			unsigned int temp;
			do
			{
				
				temp = next_rand();
			}
			while(limit && (temp >= limit));
			return min + static_cast<int>(temp / nbins);
		}
		else
		{
			return static_cast<int>(next_rand()); /*use the full range*/
		}
	}
	else
	{
		return min; /*shortcut*/
	}
}

NSISFUNC(Rand)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	pushint(rand_in_range());
}

NSISFUNC(RandMax)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const int max = popint();

	if(max < 0)
	{
		extra->exec_flags->exec_error++;
		SHOW_VERBOSE(T("RandMax() was called with bad arguments!"));
		pushstring(T("einval"));
		return;
	}
	
	pushint(rand_in_range(0, max));
}

NSISFUNC(RandMinMax)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const int max = popint();
	const int min = popint();
	
	if(min > max)
	{
		extra->exec_flags->exec_error++;
		SHOW_VERBOSE(T("RandMinMax() was called with bad arguments!"));
		pushstring(T("einval"));
		return;
	}

	pushint(rand_in_range(min, max));
}

NSISFUNC(RandList)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const int count = popint();
	const int max = popint() ;

	if((count < 1) || (count > SHRT_MAX) || (max < 0) || ((count - 1) > max))
	{
		extra->exec_flags->exec_error++;
		SHOW_VERBOSE(T("RandList() was called with bad arguments!"));
		pushstring(T("einval"));
		return;
	}

	bool *const list = new bool[max + 1];
	memset(list, 0, (max + 1) * sizeof(bool));
	
	pushstring(T("EOL"));

	int done = 0;
	while(done < count)
	{
		const int rnd = rand_in_range(0, max);
		if(!list[rnd])
		{
			pushint(rnd);
			list[rnd] = true;
			done++;
		}
	}

	memset(list, 0, (max + 1) * sizeof(bool));
	delete [] list;
}

NSISFUNC(RandBytes)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	const int count = popint();

	if((count <= 0) || (base64_encode_len((size_t)count) >= g_stringsize))
	{
		extra->exec_flags->exec_error++;
		pushstring((count > 0) ? T("too_long") : T("einval"));
		return;
	}

	BYTE *rand_buffer = new BYTE[(size_t)count];
	rand_bytes(rand_buffer, (size_t)count);

	TCHAR *rand_base64 = base64_raw2encoded(rand_buffer, (size_t)count);
	if(!rand_base64)
	{
		extra->exec_flags->exec_error++;
		pushstring(T("error"));
		DELETE_ARR(rand_buffer, BYTE, count);
		return;
	}

	pushstring(rand_base64);

	DELETE_ARR(rand_buffer, BYTE, count);
	DELETE_STR(rand_base64, 0);
}

///////////////////////////////////////////////////////////////////////////////
// STRING FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(FormatStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(fmt, g_stringsize);
	MAKESTR(out, g_stringsize);

	int v = popint();
	popstringn(fmt, 0);

	if(SNPRINTF(out, g_stringsize, fmt, v) < 0)
	{
		out[g_stringsize-1] = T('\0');
	}

	pushstring(out);

	DELETE_STR(fmt, g_stringsize);
	DELETE_STR(out, g_stringsize);
}

NSISFUNC(FormatStr2)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(fmt, g_stringsize);
	MAKESTR(out, g_stringsize);

	int v2 = popint();
	int v1 = popint();
	popstringn(fmt, 0);

	if(SNPRINTF(out, g_stringsize, fmt, v1, v2) < 0)
	{
		out[g_stringsize-1] = T('\0');
	}

	pushstring(out);

	DELETE_STR(fmt, g_stringsize);
	DELETE_STR(out, g_stringsize);
}

NSISFUNC(FormatStr3)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(fmt, g_stringsize);
	MAKESTR(out, g_stringsize);

	int v3 = popint();
	int v2 = popint();
	int v1 = popint();
	popstringn(fmt, 0);

	if(SNPRINTF(out, g_stringsize, fmt, v1, v2, v3) < 0)
	{
		out[g_stringsize-1] = T('\0');
	}

	pushstring(out);

	DELETE_STR(fmt, g_stringsize);
	DELETE_STR(out, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////

NSISFUNC(ScanStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(in, g_stringsize);
	MAKESTR(fmt, g_stringsize);

	int def = popint();
	popstringn(in, 0);
	popstringn(fmt, 0);
	int out = 0;

	if(SSCANF(in, fmt, &out) != 1)
	{
		out = def;
	}

	pushint(out);

	DELETE_STR(fmt, g_stringsize);
	DELETE_STR(in, g_stringsize);
}

NSISFUNC(ScanStr2)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(in, g_stringsize);
	MAKESTR(fmt, g_stringsize);

	int def2 = popint();
	int def1 = popint();
	popstringn(in, 0);
	popstringn(fmt, 0);
	int out1 = 0;
	int out2 = 0;
	int result = 0;

	result = SSCANF(in, fmt, &out1, &out2);
	
	if(result != 2)
	{
		if(result != 1)
		{
			out1 = def1;
		}
		out2 = def2;
	}

	pushint(out2);
	pushint(out1);

	DELETE_STR(fmt, g_stringsize);
	DELETE_STR(in, g_stringsize);
}

NSISFUNC(ScanStr3)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(in, g_stringsize);
	MAKESTR(fmt, g_stringsize);

	int def3 = popint();
	int def2 = popint();
	int def1 = popint();
	popstringn(in, 0);
	popstringn(fmt, 0);
	int out1 = 0;
	int out2 = 0;
	int out3 = 0;
	int result = 0;

	result = SSCANF(in, fmt, &out1, &out2, &out3);
	
	if(result != 3)
	{
		if(result == 0)
		{
			out1 = def1;
			out2 = def2;
		}
		else if(result == 1)
		{
			out2 = def2;
		}
		out3 = def3;
	}

	pushint(out3);
	pushint(out2);
	pushint(out1);

	DELETE_STR(fmt, g_stringsize);
	DELETE_STR(in, g_stringsize);

}

///////////////////////////////////////////////////////////////////////////////

NSISFUNC(TrimStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);
	
	popstringn(str, 0);
	pushstring(STRTRIM(str));

	DELETE_STR(str, g_stringsize);

}

NSISFUNC(TrimStrLeft)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);
	
	popstringn(str, 0);
	pushstring(STRTRIM_LEFT(str));

	DELETE_STR(str, g_stringsize);
}

NSISFUNC(TrimStrRight)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);
	
	popstringn(str, 0);
	pushstring(STRTRIM_RIGHT(str));

	DELETE_STR(str, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////

NSISFUNC(RevStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);
	
	popstringn(str, 0);

	if(str[0] != T('\0'))
	{
		size_t left = 0;
		size_t right = STRLEN(str) - 1;
		while(left < right)
		{
			TCHAR tmp = str[left];
			str[left++] = str[right];
			str[right--] = tmp;
		}
	}

	pushstring(str);
	DELETE_STR(str, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////

NSISFUNC(ValidFileName)
{
	static const TCHAR *const RESERVED = T("<>:\"/\\|?*");

	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(str, 0);

	bool valid = true;
	TCHAR last = 0x0;


	if(!str[0])
	{
		valid = false;
		goto exit209;
	}

	for(size_t i = 0; str[i]; i++)
	{
		if(ISCNTRL(str[i]))
		{
			valid = false;
			goto exit209;
		}
		for(size_t j = 0; RESERVED[j]; j++)
		{
			if(str[i] == RESERVED[j])
			{
				valid = false;
				goto exit209;
			}
		}
		last = str[i];
	}

	if((last == T(' ')) || (last == T('.')))
	{
		valid = false;
	}

exit209:
	pushstring(valid ? T("ok") : T("invalid"));
	DELETE_STR(str, g_stringsize);
}

NSISFUNC(ValidPathSpec)
{
	static const TCHAR *const RESERVED = T("<>\"|?*");

	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(str, 0);

	bool valid = true;
	TCHAR last = 0x0;


	if(!str[0])
	{
		valid = false;
		goto exit209;
	}

	for(size_t i = 0; str[i]; i++)
	{
		if(ISCNTRL(str[i]))
		{
			valid = false;
			goto exit209;
		}
		for(size_t j = 0; RESERVED[j]; j++)
		{
			if(str[i] == RESERVED[j])
			{
				valid = false;
				goto exit209;
			}
		}
		if(((i == 0) && (!ASCII_ALNUM(str[i]))) || ((i == 1) && (str[i] != T(':'))) || ((i != 1) && (str[i] == T(':'))) || ((i == 2) && (str[i] != T('/')) && (str[i] != T('\\'))))
		{
			valid = false;
			goto exit209;
		}
		last = str[i];
	}

	if((last == T(' ')) || (last == T('.')))
	{
		valid = false;
	}

exit209:
	pushstring(valid ? T("ok") : T("invalid"));
	DELETE_STR(str, g_stringsize);

}

///////////////////////////////////////////////////////////////////////////////

NSISFUNC(ValidDomainName)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);

	popstringn(str, 0);
	bool valid = true;

	const size_t len = STRLEN(str);
	if ((len < 1U) || (len > 255U))
	{
		valid = false;
		goto exit209;
	}
	
	TCHAR prev_c = T('\0');
	size_t label_size = 0U;

	for (size_t i = 0U; i < len; ++i)
	{
		const TCHAR c = str[i];
		if (c == T('.'))
		{
			if ((label_size < 1U) || (prev_c == T('-')))
			{
				valid = false;
				goto exit209;
			}
			prev_c = c;
			label_size = 0U;
			continue;
		}
		if (!(ASCII_ALNUM(c) || ((label_size > 0U) && (c == T('-')))))
		{
			valid = false;
			goto exit209;
		}
		if (++label_size > 64U)
		{
			valid = false;
			goto exit209;
		}
		prev_c = c; /*remember*/
	}

	if ((prev_c == T('-')) || (prev_c == T('.')))
	{
		valid = false;
		goto exit209;
	}

exit209:
	pushstring(valid ? T("ok") : T("invalid"));
	DELETE_STR(str, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////

NSISFUNC(StrToUtf8)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);

	popstringn(str, 0);

	BYTE *utf8_string = (BYTE*)STR_TO_UTF8(str);
	if(!utf8_string)
	{
		extra->exec_flags->exec_error++;
		pushstring(T("error"));
		goto exit209;
	}

	const size_t utf8_len = strlen((const char*)utf8_string) + 1U;
	TCHAR *base64_string = base64_raw2encoded(utf8_string, utf8_len);
	if(!base64_string)
	{
		extra->exec_flags->exec_error++;
		pushstring(T("error"));
		goto exit210;
	}

	const size_t base64_len = STRLEN(base64_string);
	if(base64_len >= g_stringsize)
	{
		extra->exec_flags->exec_error++;
		pushstring(T("too_long"));
		goto exit211;
	}

	pushstring(base64_string);

exit211:
	DELETE_STR(base64_string, base64_len);
exit210:
	DELETE_ARR(utf8_string, BYTE, utf8_len);
exit209:
	DELETE_STR(str, g_stringsize);
}

NSISFUNC(StrFromUtf8)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(str, g_stringsize);

	popstringn(str, 0);
	const int truncate = popint();

	size_t utf8_len;
	BYTE *utf8_string = base64_encoded2raw(str, utf8_len);
	if(!utf8_string)
	{
		extra->exec_flags->exec_error++;
		pushstring(T("error"));
		goto exit209;
	}

	utf8_string[utf8_len - 1U] = (BYTE)'\0';
	TCHAR *native_string = UTF8_TO_STR((const char*)utf8_string);
	if(!native_string)
	{
		extra->exec_flags->exec_error++;
		pushstring(T("error"));
		goto exit210;
	}

	const size_t native_len = STRLEN(native_string);
	if(native_len >= g_stringsize)
	{
		if(truncate > 0)
		{
			native_string[g_stringsize - 1U] = T('\0');
		}
		else
		{
			extra->exec_flags->exec_error++;
			pushstring(T("too_long"));
			goto exit211;
		}
	}

	pushstring(native_string);

exit211:
	DELETE_STR(native_string, native_len);
exit210:
	DELETE_ARR(utf8_string, char, utf8_len);
exit209:
	DELETE_STR(str, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////
// SHELL FILE FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(SHFileMove)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(from, g_stringsize);
	MAKESTR(dest, g_stringsize);

	SHFILEOPSTRUCT fileop;
	SecureZeroMemory(&fileop, sizeof(SHFILEOPSTRUCT));

	HWND hwnd = (HWND) popint();
	popstringn(dest, 0);
	popstringn(from, 0);

	fileop.hwnd = hwnd;
	fileop.wFunc = FO_MOVE;
	fileop.pFrom = from;
	fileop.pTo = dest;
	fileop.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
	if(hwnd == 0) fileop.fFlags |= FOF_SILENT;

	int result = SHFileOperation(&fileop);
	pushstring((result == 0) ? (fileop.fAnyOperationsAborted ? T("ABORTED") : T("OK")) : T("ERROR"));

	if((result != 0) && g_bStdUtilsVerbose)
	{
		TCHAR temp[1024];
		SNPRINTF(temp, 1024, T("Failed with error code: 0x%X"), result);
		temp[1023] = T('\0');
		MessageBox(NULL, temp, T("StdUtils::SHFileMove"), MB_TOPMOST|MB_ICONERROR);
	}

	DELETE_STR(from, g_stringsize);
	DELETE_STR(dest, g_stringsize);
}

NSISFUNC(SHFileCopy)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(from, g_stringsize);
	MAKESTR(dest, g_stringsize);

	SHFILEOPSTRUCT fileop;
	SecureZeroMemory(&fileop, sizeof(SHFILEOPSTRUCT));

	HWND hwnd = (HWND) popint();
	popstringn(dest, 0);
	popstringn(from, 0);

	fileop.hwnd = hwnd;
	fileop.wFunc = FO_COPY;
	fileop.pFrom = from;
	fileop.pTo = dest;
	fileop.fFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR;
	if(hwnd == 0) fileop.fFlags |= FOF_SILENT;

	int result = SHFileOperation(&fileop);
	pushstring((result == 0) ? (fileop.fAnyOperationsAborted ? T("ABORTED") : T("OK")) : T("ERROR"));

	if((result != 0) && g_bStdUtilsVerbose)
	{
		char temp[1024];
		_snprintf(temp, 1024, "Failed with error code: 0x%X", result);
		temp[1023] = '\0';
		MessageBoxA(NULL, temp, "StdUtils::SHFileCopy", MB_TOPMOST|MB_ICONERROR);
	}

	DELETE_STR(from, g_stringsize);
	DELETE_STR(dest, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////
// APPEND TO FILE
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(AppendToFile)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(from, g_stringsize);
	MAKESTR(dest, g_stringsize);

	const DWORD maxlen = static_cast<DWORD>(MAX_VAL(0, popint()));
	const DWORD offset = static_cast<DWORD>(MAX_VAL(0, popint()));
	popstringn(dest, 0);
	popstringn(from, 0);
	
	unsigned long long bytesCopied = 0;
	if(AppendToFile(from, dest, offset, maxlen, &bytesCopied))
	{
		pushint(static_cast<int>(MIN_VAL(bytesCopied, static_cast<unsigned long long>(INT_MAX))));
	}
	else
	{
		pushstring(T("error"));
	}

	DELETE_STR(from, g_stringsize);
	DELETE_STR(dest, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////
// EXEC SHELL AS USER
///////////////////////////////////////////////////////////////////////////////

#ifndef STDUTILS_TINY_MODE

NSISFUNC(ExecShellAsUser)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(file, g_stringsize);
	MAKESTR(verb, g_stringsize);
	MAKESTR(args, g_stringsize);

	popstringn(args, 0);
	popstringn(verb, 0);
	popstringn(file, 0);
	
	if (!file[0]) DELETE_STR(file, g_stringsize);
	if (!verb[0]) DELETE_STR(verb, g_stringsize);
	if (!args[0]) DELETE_STR(args, g_stringsize);

	if(!(file))
	{
		pushstring(T("einval"));
		goto exit209;
	}

	int result = ShellExecAsUser(verb, file, args, hWndParent, true);
	
	switch(result)
	{
	case SHELL_EXEC_AS_USER_SUCCESS:
		pushstring(T("ok"));
		break;
	case SHELL_EXEC_AS_USER_FAILED:
		pushstring(T("error"));
		break;
	case SHELL_EXEC_AS_USER_TIMEOUT:
		pushstring(T("timeout"));
		break;
	case SHELL_EXEC_AS_USER_UNSUPPORTED:
		pushstring(T("unsupported"));
		break;
	case SHELL_EXEC_AS_USER_FALLBACK:
		pushstring(T("fallback"));
		break;
	case SHELL_EXEC_AS_USER_NOT_FOUND:
		pushstring(T("not_found"));
		break;
	default:
		pushstring(T("unknown"));
		break;
	}

exit209:
	DELETE_STR(verb, g_stringsize);
	DELETE_STR(file, g_stringsize);
	DELETE_STR(args, g_stringsize);
}

#endif //STDUTILS_TINY_MODE

///////////////////////////////////////////////////////////////////////////////
// INVOKE SHELL VERB
///////////////////////////////////////////////////////////////////////////////

#ifndef STDUTILS_TINY_MODE

NSISFUNC(InvokeShellVerb)
{
	static const DWORD ID_LUT[][3] =
	{
		{ 5386, MAXDWORD },
		{ 5387, MAXDWORD },
		{ 51201, 5381, MAXDWORD },
		{ 51394, 5382, MAXDWORD }
	};

	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(file, g_stringsize);

	const int verb = popint();
	popstringn(file, 0);
	popstringn(path, 0);
	
	if (!file[0]) DELETE_STR(file, g_stringsize);
	if (!path[0]) DELETE_STR(path, g_stringsize);

	if(!(file && path))
	{
		SHOW_VERBOSE(T("InvokeShellVerb: Specified file name and/or path is missing!"));
		pushstring(T("einval"));
		goto exit209;
	}

	if((verb < 0) || (verb > 3))
	{
		SHOW_VERBOSE(T("InvokeShellVerb: And invalid verb id has been specified!"));
		pushstring(T("einval"));
		goto exit209;
	}

	int result = MyInvokeShellVerb(path, file, ID_LUT[verb], true);
	
	switch(result)
	{
	case INVOKE_SHELLVERB_SUCCESS:
		pushstring(T("ok"));
		break;
	case INVOKE_SHELLVERB_FAILED:
		pushstring(T("error"));
		break;
	case INVOKE_SHELLVERB_TIMEOUT:
		pushstring(T("timeout"));
		break;
	case INVOKE_SHELLVERB_UNSUPPORTED:
		pushstring(T("unsupported"));
		break;
	case INVOKE_SHELLVERB_NOT_FOUND:
		pushstring(T("not_found"));
		break;
	default:
		pushstring(T("unknown"));
		break;
	}

exit209:
	DELETE_STR(path, g_stringsize);
	DELETE_STR(file, g_stringsize);
}

#endif //STDUTILS_TINY_MODE

///////////////////////////////////////////////////////////////////////////////
// EXEC SHELL WAIT
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(ExecShellWaitEx)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(file, g_stringsize);
	MAKESTR(verb, g_stringsize);
	MAKESTR(args, g_stringsize);
	
	popstringn(args, 0);
	popstringn(verb, 0);
	popstringn(file, 0);

	SHELLEXECUTEINFO shInfo;
	memset(&shInfo, 0, sizeof(SHELLEXECUTEINFO));
	shInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	shInfo.hwnd = hWndParent;
	shInfo.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;
	shInfo.lpFile = file;
	shInfo.lpVerb = (_tcslen(verb) > 0) ? verb : NULL;
	shInfo.lpParameters = (_tcslen(args) > 0) ? args : NULL;
	shInfo.nShow = SW_SHOWNORMAL;

	if(ShellExecuteEx(&shInfo) != FALSE)
	{
		if(VALID_HANDLE(shInfo.hProcess))
		{
			TCHAR out[32];
			SNPRINTF(out, 32, T("hProc:%08X"), shInfo.hProcess);
			pushstring(out);
			pushstring(_T("ok"));
		}
		else
		{
			pushint(0);
			pushstring(T("no_wait"));
		}
	}
	else
	{
		pushint(GetLastError());
		pushstring(T("error"));
	}

	DELETE_STR(file, g_stringsize);
	DELETE_STR(verb, g_stringsize);
	DELETE_STR(args, g_stringsize);
}

NSISFUNC(WaitForProcEx)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(temp, g_stringsize);
	popstringn(temp, 0);

	HANDLE hProc = NULL;
	int result = SSCANF(temp, T("hProc:%X"), &hProc);

	DWORD dwExitCode = 0;
	bool success = false;

	if(result == 1)
	{
		if(hProc != NULL)
		{
			if(WaitForSingleObject(hProc, INFINITE) == WAIT_OBJECT_0)
			{
				success = (GetExitCodeProcess(hProc, &dwExitCode) != FALSE);
			}
			CloseHandle(hProc);
		}
	}

	if(success)
	{
		pushint(dwExitCode);
	}
	else
	{
		pushstring(T("error"));
	}

	DELETE_STR(temp, g_stringsize);
}


///////////////////////////////////////////////////////////////////////////////
// GET COMMAND-LINE PARAMS
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(GetParameter)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(aval, g_stringsize);
	MAKESTR(name, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(aval, 0);
	popstringn(name, 0);

	commandline_get_arg(STRTRIM(name), aval, g_stringsize);
	pushstring(STRTRIM(aval));

	DELETE_STR(aval, g_stringsize);
	DELETE_STR(name, g_stringsize);
}

NSISFUNC(TestParameter)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(name, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(name, 0);

	pushstring(commandline_get_arg(STRTRIM(name), NULL, 0) ? T("true") : T("false"));

	DELETE_STR(name, g_stringsize);
}

NSISFUNC(ParameterStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(value, g_stringsize);
	SETLOCALE(0, T("C"));

	const int index = popint();
	if(commandline_get_raw(index, value, g_stringsize))
	{
		pushstring(STRTRIM(value));
	}
	else
	{
		pushstring(T("error"));
	}

	DELETE_STR(value, g_stringsize);
}

NSISFUNC(ParameterCnt)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	SETLOCALE(0, T("C"));

	const int index = commandline_get_cnt();
	if(index >= 0)
	{
		pushint(index);
		return;
	}

	pushstring(T("error"));
}

NSISFUNC(GetAllParameters)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	SETLOCALE(0, T("C"));

	int truncate = popint();
	const TCHAR *cmd = commandline_get_all();

	if((STRLEN(cmd) < g_stringsize) || truncate)
	{
		pushstring(cmd);
	}
	else
	{
		pushstring(T("too_long"));
	}
}

///////////////////////////////////////////////////////////////////////////////
// GET REAL OS VERSION
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(GetRealOsVersion)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	bool flag;
	unsigned int version[3];

	if(get_real_os_version(&version[0], &version[1], &version[2], &flag))
	{
		pushint(version[2]);
		pushint(version[1]);
		pushint(version[0]);
	}
	else
	{
		pushstring(T("error"));
		pushstring(T("error"));
		pushstring(T("error"));
	}
}

NSISFUNC(GetRealOsBuildNo)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	bool flag;
	unsigned int buildNumber;

	if(get_real_os_buildNo(&buildNumber, &flag))
	{
		pushint(buildNumber);
	}
	else
	{
		pushstring(T("error"));
	}
}

NSISFUNC(VerifyRealOsVersion)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	bool flag;
	unsigned int expectedVersion[3];
	unsigned int detectedVersion[3];

	expectedVersion[2] = abs(popint());
	expectedVersion[1] = abs(popint());
	expectedVersion[0] = abs(popint());

	if(!get_real_os_version(&detectedVersion[0], &detectedVersion[1], &detectedVersion[2], &flag))
	{
		pushstring(T("error"));
		return;
	}

	//Majaor version
	for(size_t i = 0; i < 3; i++)
	{
		if(detectedVersion[i] > expectedVersion[i])
		{
			pushstring(T("newer"));
			return;
		}
		if(detectedVersion[i] < expectedVersion[i])
		{
			pushstring(T("older"));
			return;
		}
	}

	pushstring(T("ok"));
}

NSISFUNC(VerifyRealOsBuildNo)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	bool flag;
	unsigned int expectedBuildNo;
	unsigned int detectedBuildNo;

	expectedBuildNo = abs(popint());
	
	if(!get_real_os_buildNo(&detectedBuildNo, &flag))
	{
		pushstring(T("error"));
		return;
	}

	if(detectedBuildNo > expectedBuildNo)
	{
		pushstring(T("newer"));
		return;
	}
	if(detectedBuildNo < expectedBuildNo)
	{
		pushstring(T("older"));
		return;
	}

	pushstring(T("ok"));
}

NSISFUNC(GetRealOsName)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	bool flag;
	unsigned int detectedVersion[3];

	if(!get_real_os_version(&detectedVersion[0], &detectedVersion[1], &detectedVersion[2], &flag))
	{
		pushstring(T("error"));
		return;
	}

	if(!get_os_server_edition(&flag))
	{
		flag = false;
	}

	pushstring(get_os_friendly_name(detectedVersion[0], detectedVersion[1], flag));
}

NSISFUNC(GetOsEdition)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	bool isServerEdition;
	if(!get_os_server_edition(&isServerEdition))
	{
		pushstring(T("error"));
		return;
	}

	pushstring(isServerEdition ? T("server") : T("workstation"));
}

NSISFUNC(GetOsReleaseId)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	DWORD releaseId;
	const int result = get_os_release_id(&releaseId);
	if(result > 0)
	{
		pushint((releaseId > DWORD(INT_MAX)) ? INT_MAX : static_cast<int>(releaseId));
	}
	else
	{
		pushstring((result < 0) ? T("error") : T("unavail"));
	}
}

NSISFUNC(GetOsReleaseName)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	DWORD releaseId;
	const int result = get_os_release_id(&releaseId);
	if(result > 0)
	{
		pushstring(get_os_release_name(releaseId)); /*translate*/
	}
	else
	{
		pushstring((result < 0) ? T("error") : T("unavail"));
	}
}

///////////////////////////////////////////////////////////////////////////////
// HASH COMPUTATION
///////////////////////////////////////////////////////////////////////////////

#ifndef STDUTILS_TINY_MODE

static int GetHashType(const TCHAR *const type)
{
	static const struct
	{
		const TCHAR *const name;
		const int id;
	}
	HASH_ALGO_MAPPING[] =
	{
		{ T("CRC-32"),     STD_HASHTYPE_CRC_32   },
		{ T("MD5-128"),    STD_HASHTYPE_MD5_128  },
		{ T("SHA1-160"),   STD_HASHTYPE_SHA1_160 },
		{ T("SHA2-224"),   STD_HASHTYPE_SHA2_224 },
		{ T("SHA2-256"),   STD_HASHTYPE_SHA2_256 },
		{ T("SHA2-384"),   STD_HASHTYPE_SHA2_384 },
		{ T("SHA2-512"),   STD_HASHTYPE_SHA2_512 },
		{ T("SHA3-224"),   STD_HASHTYPE_SHA3_224 },
		{ T("SHA3-256"),   STD_HASHTYPE_SHA3_256 },
		{ T("SHA3-384"),   STD_HASHTYPE_SHA3_384 },
		{ T("SHA3-512"),   STD_HASHTYPE_SHA3_512 },
		{ T("BLAKE2-224"), STD_HASHTYPE_BLK2_224 },
		{ T("BLAKE2-256"), STD_HASHTYPE_BLK2_256 },
		{ T("BLAKE2-384"), STD_HASHTYPE_BLK2_384 },
		{ T("BLAKE2-512"), STD_HASHTYPE_BLK2_512 },
		{ NULL, -1 }
	};

	for(size_t i = 0; HASH_ALGO_MAPPING[i].name; i++)
	{
		if(STRICMP(type, HASH_ALGO_MAPPING[i].name) == 0)
		{
			return HASH_ALGO_MAPPING[i].id;
		}
	}

	SHOW_VERBOSE(T("And invalid hash function has been specified!"));
	return -1;
}

NSISFUNC(HashFile)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(file, g_stringsize);
	MAKESTR(temp, g_stringsize);

	popstringn(file, 0);
	popstringn(temp, 0);

	const int hashType = GetHashType(STRTRIM(temp));
	if(hashType >= 0)
	{
		if(ComputeHash_FromFile(hashType, file, temp, g_stringsize))
		{
			pushstring(temp);
		}
		else
		{
			pushstring(T("error"));
		}
	}
	else
	{
		pushstring(T("einval"));
	}

	DELETE_STR(file, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

NSISFUNC(HashText)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(temp, g_stringsize);
	MAKESTR(text, g_stringsize);

	popstringn(text, 0);
	popstringn(temp, 0);

	const int hashType = GetHashType(STRTRIM(temp));
	if(hashType >= 0)
	{
		if(ComputeHash_FromText(hashType, text, temp, g_stringsize))
		{
			pushstring(temp);
		}
		else
		{
			pushstring(T("error"));
		}
	}
	else
	{
		pushstring(T("einval"));
	}

	DELETE_STR(text, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

#endif //STDUTILS_TINY_MODE

///////////////////////////////////////////////////////////////////////////////
// PATH UTILITY FUNCTIONS
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(NormalizePath)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(temp, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	if(Path_Normalize(path, temp, g_stringsize))
	{
		pushstring(temp);
	}
	else
	{
		pushstring(T(""));
	}

	DELETE_STR(path, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

NSISFUNC(GetParentPath)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(temp, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	if(Path_GetParent(path, temp, g_stringsize))
	{
		pushstring(temp);
	}
	else
	{
		pushstring(T(""));
	}

	DELETE_STR(path, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

NSISFUNC(SplitPath)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(out1, g_stringsize);
	MAKESTR(out2, g_stringsize);
	MAKESTR(out3, g_stringsize);
	MAKESTR(out4, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	Path_Split(path, out1, out2, out3, out4);

	pushstring(out4);
	pushstring(out3);
	pushstring(out2);
	pushstring(out1);

	DELETE_STR(path, g_stringsize);
	DELETE_STR(out1, g_stringsize);
	DELETE_STR(out2, g_stringsize);
	DELETE_STR(out3, g_stringsize);
	DELETE_STR(out4, g_stringsize);
}

NSISFUNC(GetDrivePart)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(temp, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	Path_Split(path, temp, NULL, NULL, NULL);
	pushstring(temp);

	DELETE_STR(path, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

NSISFUNC(GetDirectoryPart)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(temp, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	Path_Split(path, NULL, temp, NULL, NULL);
	pushstring(temp);

	DELETE_STR(path, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

NSISFUNC(GetFileNamePart)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(temp, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	Path_Split(path, NULL, NULL, temp, NULL);
	pushstring(temp);

	DELETE_STR(path, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

NSISFUNC(GetExtensionPart)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(path, g_stringsize);
	MAKESTR(temp, g_stringsize);
	SETLOCALE(0, T("C"));

	popstringn(path, 0);

	Path_Split(path, NULL, NULL, NULL, temp);
	pushstring(temp);

	DELETE_STR(path, g_stringsize);
	DELETE_STR(temp, g_stringsize);
}

///////////////////////////////////////////////////////////////////////////////
// CREATE/DESTROY TIMER
///////////////////////////////////////////////////////////////////////////////

#ifndef STDUTILS_TINY_MODE

NSISFUNC(TimerCreate)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();

	const int interval = popint();
	const int procAddress = popint();

	UINT_PTR id;
	if(timer_create(procAddress, interval, hWndParent, extra, id))
	{
		TCHAR out[32];
		SNPRINTF(out, 32, T("TimerId:%08X"), id);
		pushstring(out);
	}
	else
	{
		pushstring(T("error"));
	}
}

NSISFUNC(TimerDestroy)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(temp, g_stringsize);

	popstringn(temp, 0);
	UINT_PTR id;
	bool success = false;

	if(SSCANF(temp, T("TimerId:%X"), &id) == 1)
	{
		success = timer_destroy(id);
	}

	pushstring(success ? T("ok") : T("error"));
	DELETE_STR(temp, g_stringsize);
}

#endif //STDUTILS_TINY_MODE

///////////////////////////////////////////////////////////////////////////////
// DPAPI SUPPORT
///////////////////////////////////////////////////////////////////////////////

#ifndef STDUTILS_TINY_MODE

NSISFUNC(ProtectStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(pscp, g_stringsize);
	MAKESTR(salt, g_stringsize);
	MAKESTR(data, g_stringsize);

	popstringn(data, 0);
	popstringn(salt, 0);
	popstringn(pscp, 0);

	bool machine_scope = false;
	if (!((STRICMP(pscp, T("CU")) == 0) || (machine_scope = (STRICMP(pscp, T("LM")) == 0))))
	{
		SHOW_VERBOSE(T("ProtectStr() was called with bad 'scope' value!"));
		extra->exec_flags->exec_error++;
		pushstring(T("einval"));
		goto exit209;
	}

	TCHAR *protected_str;
	const int ret = dpapi_protect_text(&protected_str, data, salt[0] ? salt : NULL, machine_scope);

	if ((ret <= 0) || (!protected_str))
	{
		extra->exec_flags->exec_error++;
		pushstring((ret < 0) ? T("encr_failed") : T("error"));
		goto exit209;
	}

	const size_t len = STRLEN(protected_str);
	if (len < g_stringsize)
	{
		pushstring(protected_str);
	}
	else
	{
		extra->exec_flags->exec_error++;
		pushstring(T("too_long"));
	}

	DELETE_STR(protected_str, len);

exit209:
	DELETE_STR(pscp, g_stringsize);
	DELETE_STR(salt, g_stringsize);
	DELETE_STR(data, g_stringsize);
}

NSISFUNC(UnprotectStr)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	MAKESTR(salt, g_stringsize);
	MAKESTR(data, g_stringsize);

	popstringn(data, 0);
	popstringn(salt, 0);
	const int truncate = popint();

	if (!data[0])
	{
		SHOW_VERBOSE(T("UnprotectStr() was called with empty 'data' value!"));
		extra->exec_flags->exec_error++;
		pushstring(T("einval"));
		goto exit209;
	}

	TCHAR *palintext_str;
	const int ret = dpapi_unprotect_text(&palintext_str, data, salt[0] ? salt : NULL);

	if ((ret <= 0) || (!palintext_str))
	{
		extra->exec_flags->exec_error++;
		pushstring((ret < 0) ? T("decr_failed") : T("error"));
		goto exit209;
	}

	const size_t len = STRLEN(palintext_str);
	if (len < g_stringsize)
	{
		pushstring(palintext_str);
	}
	else
	{
		if (truncate > 0)
		{
			palintext_str[g_stringsize - 1] = T('\0');
			pushstring(palintext_str);
		}
		else
		{
			extra->exec_flags->exec_error++;
			pushstring(T("too_long"));
		}
	}

	DELETE_STR(palintext_str, len);

exit209:
	DELETE_STR(salt, g_stringsize);
	DELETE_STR(data, g_stringsize);
}

#endif //STDUTILS_TINY_MODE

///////////////////////////////////////////////////////////////////////////////
// FOR DEBUGGING
///////////////////////////////////////////////////////////////////////////////

NSISFUNC(SetVerboseMode)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	g_bStdUtilsVerbose = (popint() > 0);
}

///////////////////////////////////////////////////////////////////////////////

#include "resource.h"

static const TCHAR *dllTimeStamp = T(__DATE__) T(", ") T(__TIME__);
static const TCHAR *dllVerString = T(DLL_VERSION_STRING);

NSISFUNC(GetLibVersion)
{
	EXDLL_INIT();
	REGSITER_CALLBACK();
	pushstring(dllTimeStamp);
	pushstring(dllVerString);
}
