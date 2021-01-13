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

#pragma once

#include "PathUtils.h"
#include "UnicodeSupport.h"

#define IS_SEPARATOR(X) (((X) == T('/')) || ((X) == T('\\')))
#define IS_DRIVEROOT(X) (ASCII_ALNUM((X)[0]) && ((X)[1] == T(':')) && ((X)[2] == T('\0')))

static bool trim_trailing_component(TCHAR *const str);
static bool trim_trailing_separator(TCHAR *const str);

//===========================================================================
// PUBLIC FUNCTIONS
//===========================================================================

bool Path_Normalize(const TCHAR *const path, TCHAR *const normalized, const size_t buff_size)
{
	if (path && path[0] && normalized && (buff_size > 0))
	{
		const size_t len = GetFullPathName(path, buff_size, normalized, NULL);
		if ((len > 0) && (len < buff_size))
		{
			trim_trailing_separator(normalized);
			if ((buff_size > 3) && IS_DRIVEROOT(normalized))
			{
				normalized[2] = T('\\');
				normalized[3] = T('\0');
			}
			if (normalized[0])
			{
				return true; /*successful*/
			}
		}
	}
	return false;
}

bool Path_GetParent(const TCHAR *const path, TCHAR *const parentPath, const size_t buff_size)
{
	if (path && path[0] && parentPath && (buff_size > 0))
	{
		if (Path_Normalize(path, parentPath, buff_size))
		{
			if(trim_trailing_component(parentPath))
			{
				trim_trailing_separator(parentPath);
				if ((buff_size > 3) && IS_DRIVEROOT(parentPath))
				{
					parentPath[2] = T('\\');
					parentPath[3] = T('\0');
				}
				if (parentPath[0])
				{
					return true; /*successful*/
				}
			}
		}
	}
	return false;
}

void Path_Split(const TCHAR *const path, TCHAR *const drive, TCHAR *const dir, TCHAR *const fname, TCHAR *const ext)
{
	SPLITPATH(path, drive, dir, fname, ext);
}

//===========================================================================
// INTERNAL FUNCTIONS
//===========================================================================

static bool trim_trailing_separator(TCHAR *const str)
{
	bool modifiedStr = false;
	if (str && str[0])
	{
		size_t len = STRLEN(str);
		while (len > 0)
		{
			const TCHAR c = str[--len];
			if (!IS_SEPARATOR(c))
			{
				break; /*no more separators*/
			}
			str[len] = T('\0');
			modifiedStr = true;
		}
	}
	return modifiedStr;
}

static bool trim_trailing_component(TCHAR *const str)
{
	bool modifiedStr = false;
	if (str && str[0])
	{
		size_t len = STRLEN(str);
		while (len > 0)
		{
			const TCHAR c = str[--len];
			if (IS_SEPARATOR(c))
			{
				break; /*separator detected*/
			}
			str[len] = T('\0');
			modifiedStr = true;
		}
	}
	return modifiedStr;
}
