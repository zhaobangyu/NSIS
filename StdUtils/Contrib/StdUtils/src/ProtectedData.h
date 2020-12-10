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

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#endif

#ifndef STDUTILS_TINY_MODE

int dpapi_protect_data(BYTE **const protected_out, size_t *const protected_len, const BYTE *const plaintext_in, const size_t plaintext_len, const BYTE *const salt_in = NULL, const size_t salt_len = 0U, const bool machine_scope = false);
int dpapi_unprotect_data(BYTE **const plaintext_out, size_t *const plaintext_len, const BYTE *const protected_in, const size_t protected_len, const BYTE *const salt_in = NULL, const size_t salt_len = 0U);

int dpapi_protect_text(TCHAR **const protected_out, const TCHAR *const plaintext_in, const TCHAR *const salt_in = NULL, const bool machine_scope = false);
int dpapi_unprotect_text(TCHAR **const plaintext_out, const TCHAR *const protected_in, const TCHAR *const salt_in = NULL);

#endif //STDUTILS_TINY_MODE
