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

#include "StdUtils.h"

static const size_t MAX_DRIVE =   3;
static const size_t MAX_DIR   = 256;
static const size_t MAX_FNAME = 256;
static const size_t MAX_EXT   = 256;

bool Path_Normalize(const TCHAR *const path, TCHAR *const normalized, const size_t buff_size);
bool Path_GetParent(const TCHAR *const path, TCHAR *const normalized, const size_t buff_size);
void Path_Split(const TCHAR *const path, TCHAR *const drive, TCHAR *const dir, TCHAR *const fname, TCHAR *const ext);
