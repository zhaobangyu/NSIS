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

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

//-----------------------------------------------------------------------------
// UTILITIES
//-----------------------------------------------------------------------------

static void release_wstr(wchar_t *const str)
{
	if (str)
	{
		const size_t len = wcslen(str);
		if (len > 0)
		{
			SecureZeroMemory(str, len * sizeof(wchar_t));
		}
		delete[] str;
	}
}

//-----------------------------------------------------------------------------
// CHARSET CONVERSION FUNCTIONS
//-----------------------------------------------------------------------------

static char *wide_char_to_multi_byte(const wchar_t *const input, const UINT cp)
{
	char *Buffer;
	int BuffSize, Result;
	BuffSize = WideCharToMultiByte(cp, 0, input, -1, NULL, 0, NULL, NULL);
	if (BuffSize > 0)
	{
		Buffer = new char[BuffSize];
		Result = WideCharToMultiByte(cp, 0, input, -1, Buffer, BuffSize, NULL, NULL);
		if ((Result > 0) && (Result <= BuffSize))
		{
			return Buffer;
		}
		SecureZeroMemory(Buffer, BuffSize * sizeof(char));
		delete[] Buffer;
	}
	return NULL;
}

wchar_t *multi_byte_to_wide_char(const char *const input, const UINT cp)
{
	wchar_t *Buffer;
	int BuffSize, Result;
	BuffSize = MultiByteToWideChar(cp, 0, input, -1, NULL, 0);
	if (BuffSize > 0)
	{
		Buffer = new wchar_t[BuffSize];
		Result = MultiByteToWideChar(cp, 0, input, -1, Buffer, BuffSize);
		if ((Result > 0) && (Result <= BuffSize))
		{
			return Buffer;
		}
		SecureZeroMemory(Buffer, BuffSize * sizeof(wchar_t));
		delete[] Buffer;
	}
	return NULL;
}

char *utf16_to_utf8(const wchar_t *const input)
{
	return wide_char_to_multi_byte(input, CP_UTF8);
}

char *utf16_to_ansi(const wchar_t *const input)
{
	return wide_char_to_multi_byte(input, CP_ACP);
}

wchar_t *utf8_to_utf16(const char *const input)
{
	return multi_byte_to_wide_char(input, CP_UTF8);
}

wchar_t *ansi_to_utf16(const char *const input)
{
	return multi_byte_to_wide_char(input, CP_ACP);
}

char *ansi_to_utf8(const char *const input)
{
	wchar_t *const utf16_str = multi_byte_to_wide_char(input, CP_ACP);
	if (utf16_str)
	{
		char *const utf8_str = wide_char_to_multi_byte(utf16_str, CP_UTF8);
		release_wstr(utf16_str);
		return utf8_str;
	}
	return NULL;
}

char *utf8_to_ansi(const char *const input)
{
	wchar_t *const utf16_str = multi_byte_to_wide_char(input, CP_UTF8);
	if (utf16_str)
	{
		char *const ansi_str = wide_char_to_multi_byte(utf16_str, CP_ACP);
		release_wstr(utf16_str);
		return ansi_str;
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// TRIM FUNCTIONS (ANSI)
//-----------------------------------------------------------------------------

inline bool str_whitespace(const char c)
{
	return (c) && (iscntrl(c) || isspace(c));	//return (c == ' ') || (c == '\t') || (c == '\n') || (c == '\r');
}

const char *strtrim_left(const char *const input)
{
	size_t left = 0;
	if(input[0] != L'\0')
	{
		while(str_whitespace(input[left])) left++;
	}
	return &input[left];
}

char *strtrim_right(char *const input)
{
	if(input[0] != L'\0')
	{
		size_t right = strlen(input);
		while(right > 0)
		{
			if(!str_whitespace(input[--right])) break;
			input[right] = L'\0';
		}
	}
	return input;
}

char *strtrim(char *const input)
{
	return strtrim_right((char*)strtrim_left(input));
}

//-----------------------------------------------------------------------------
// TRIM FUNCTIONS (UNICODE)
//-----------------------------------------------------------------------------

inline bool wcs_whitespace(const wchar_t c)
{
	return (c) && (iswcntrl(c) || iswspace(c));	//return (c == L' ') || (c == L'\t') || (c == L'\n') || (c == L'\r');
}

const wchar_t *wcstrim_left(const wchar_t *const input)
{
	size_t left = 0;
	if(input[0] != L'\0')
	{
		while(wcs_whitespace(input[left])) left++;
	}
	return &input[left];
}

wchar_t *wcstrim_right(wchar_t *const input)
{
	if(input[0] != L'\0')
	{
		size_t right = wcslen(input);
		while(right > 0)
		{
			if(!wcs_whitespace(input[--right])) break;
			input[right] = L'\0';
		}
	}
	return input;
}

wchar_t *wcstrim(wchar_t *const input)
{
	return wcstrim_right((wchar_t*)wcstrim_left(input));
}

//-----------------------------------------------------------------------------
// CHARSET TEST
//-----------------------------------------------------------------------------

bool str_ascii_alpha(const char c)
{
	return ((c >= 'a') && (c <= 'z'))
		|| ((c >= 'A') && (c <= 'Z'));
}

bool wcs_ascii_alpha(const wchar_t c)
{
	return ((c >= L'a') && (c <= L'z'))
		|| ((c >= L'A') && (c <= L'Z'));
}

bool str_ascii_alnum(const char c)
{
	return str_ascii_alpha(c)
		|| ((c >= '0') && (c <= '9'));
}

bool wcs_ascii_alnum(const wchar_t c)
{
	return wcs_ascii_alpha(c)
		|| ((c >= L'0') && (c <= L'9'));
}
