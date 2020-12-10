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

#include "ProtectedData.h"
#ifndef STDUTILS_TINY_MODE

#include "Base64.h"
#include "RandUtils.h"
#include "UnicodeSupport.h"

#include <WinCrypt.h>
//#include <Dpapi.h>

#define FREE_TEXT(X) free_buffer((X), ((X) && ((X)[0])) ? (strlen((const char*)(X))) : 0U)

static const DWORD DEFAULT_SALT_LEN = 64;

//-----------------------------------------------------------------------------
// UTILITY FUNCTIONS
//-----------------------------------------------------------------------------

static inline void free_buffer(const BYTE *const buffer, const size_t len)
{
	if (buffer)
	{
		if (len > 0U)
		{
			SecureZeroMemory((void*)buffer, len * sizeof(BYTE));
		}
		delete[] buffer;
	}
}

static inline void free_blob(DATA_BLOB *const blob)
{
	if (blob && blob->pbData)
	{
		if (blob->cbData > 0U)
		{
			SecureZeroMemory(blob->pbData, blob->cbData * sizeof(BYTE));
		}
		LocalFree(blob->pbData);
		blob->pbData = NULL;
	}
}

static inline void free_text(char *const text)
{
	if (text)
	{
		if (text[0])
		{
			const size_t len = strlen(text);
			if (len > 0U)
			{
				SecureZeroMemory(text, len * sizeof(char));
			}
			
		}
		delete [] text;
	}
}

static inline BYTE *copy_buffer(const BYTE *const source, const size_t len)
{
	BYTE *result = NULL;
	if (source && (len > 0U))
	{
		result = new BYTE[len];
		memcpy(result, source, len);
	}
	return result;
}

static inline BYTE *concat_buffers(const BYTE *const src1, const size_t len1, const BYTE *const src2, const size_t len2)
{
	BYTE *result = NULL;
	if ((src1 || src2) && ((len1 > 0U) || (len2 > 0U)))
	{
		result = new BYTE[len1 + len2];
		if (src1 && (len1 > 0U))
		{
			memcpy(result, src1, len1);
		}
		if (src2 && (len2 > 0U))
		{
			memcpy(&result[len1], src2, len2);
		}
	}
	return result;
}

//-----------------------------------------------------------------------------
// DPAPI WRAPPER
//-----------------------------------------------------------------------------

int dpapi_protect_data(BYTE **const protected_out, size_t *const protected_len, const BYTE *const plaintext_in, const size_t plaintext_len, const BYTE *const salt_in, const size_t salt_len, const bool machine_scope)
{
	*protected_len = 0U;
	*protected_out = NULL;

	//Check given parameters
	if ((!plaintext_in) || (plaintext_len < 1U))
	{
		return 0;
	}
	
	//Use given salt or generate new random salt
	size_t salt_buff_len;
	BYTE *salt_buffer;
	if (salt_in && (salt_len > 0U))
	{
		if (!(salt_buffer = copy_buffer(salt_in, salt_buff_len = salt_len)))
		{
			return 0;
		}
	}
	else
	{
		salt_buffer = new BYTE[DEFAULT_SALT_LEN];
		rand_bytes(salt_buffer, salt_buff_len = DEFAULT_SALT_LEN);
	}

	//Actually protect the given data!
	DATA_BLOB blobIn = { plaintext_len, (BYTE*)plaintext_in }, blobEntropy = { salt_buff_len, salt_buffer }, blobOut = {};
	const DWORD scope_value = machine_scope ? CRYPTPROTECT_LOCAL_MACHINE : 0U;
	if (!CryptProtectData(&blobIn, NULL, &blobEntropy, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN | scope_value, &blobOut))
	{
		free_buffer(salt_buffer, salt_buff_len);
		return (-1);
	}

	//Concatenate generated salt (if any) and protected data
	size_t result_buff_len;
	BYTE *result_buffer;
	if (salt_in && (salt_len >= 0))
	{
		if (!(result_buffer = copy_buffer(blobOut.pbData, result_buff_len = blobOut.cbData)))
		{
			free_buffer(salt_buffer, salt_buff_len);
			free_blob(&blobOut);
			return 0;
		}
	}
	else
	{
		result_buff_len = salt_buff_len + blobOut.cbData;
		if (!(result_buffer = concat_buffers(salt_buffer, salt_buff_len, blobOut.pbData, blobOut.cbData)))
		{
			free_buffer(salt_buffer, salt_buff_len);
			free_blob(&blobOut);
			return 0;
		}
	}

	//Clean up
	free_buffer(salt_buffer, salt_buff_len);
	free_blob(&blobOut);
	
	//Return
	*protected_out = result_buffer;
	*protected_len = result_buff_len;
	return 1;
}

int dpapi_unprotect_data(BYTE **const plaintext_out, size_t *const plaintext_len, const BYTE *const protected_in, const size_t protected_len, const BYTE *const salt_in, const size_t salt_len)
{
	*plaintext_len = 0U;
	*plaintext_out = NULL;

	//Check given parameters
	if ((!protected_in) || (protected_len < 1U) || ((protected_len <= DEFAULT_SALT_LEN) && ((!salt_in) || (salt_len <= 0U))))
	{
		return 0;
	}

	//Select salt and data
	const BYTE *salt_buffer, *protected_buffer;
	size_t salt_buff_len, protected_buff_len;
	if (salt_in && (salt_len > 0U))
	{
		salt_buffer = salt_in;
		salt_buff_len = salt_len;
		protected_buffer = protected_in;
		protected_buff_len = protected_len;
	}
	else
	{
		salt_buffer = protected_in;
		salt_buff_len = DEFAULT_SALT_LEN;
		protected_buffer = &protected_in[DEFAULT_SALT_LEN];
		protected_buff_len = protected_len - DEFAULT_SALT_LEN;
	}

	//Actually unprotect the given data!
	DATA_BLOB blobIn = { protected_buff_len, (BYTE*)protected_buffer }, blobEntropy = { salt_buff_len, (BYTE*)salt_buffer }, blobOut = {};
	if (!CryptUnprotectData(&blobIn, NULL, &blobEntropy, NULL, NULL, CRYPTPROTECT_UI_FORBIDDEN, &blobOut))
	{
		return (-1);
	}

	//Copy result data
	BYTE *const result_buffer = copy_buffer(blobOut.pbData, blobOut.cbData);
	const size_t result_buff_len = blobOut.cbData;
	if (!result_buffer)
	{
		free_blob(&blobOut);
	}

	//Clean up
	free_blob(&blobOut);

	//Return
	*plaintext_out = result_buffer;
	*plaintext_len = result_buff_len;
	return 1;
}

//-----------------------------------------------------------------------------
// TEXT (BASE64) SUPPORT
//-----------------------------------------------------------------------------

int dpapi_protect_text(TCHAR **const protected_out, const TCHAR *const plaintext_in, const TCHAR *const salt_in, const bool machine_scope)
{
	*protected_out = NULL;

	//Check given parameters
	if (!plaintext_in)
	{
		return 0;
	}

	//Convert given text to bytes
	const BYTE *const plaintext_buffer = (const BYTE*)STR_TO_UTF8(plaintext_in);
	if (!plaintext_buffer)
	{
		return 0;
	}

	//Convert given salt to bytes, if present
	const BYTE *salt_buffer = NULL;
	if (salt_in && salt_in[0])
	{
		salt_buffer = (const BYTE*)STR_TO_UTF8(salt_in);
		if ((!salt_buffer) || (!salt_buffer[0]))
		{
			FREE_TEXT(plaintext_buffer);
			FREE_TEXT(salt_buffer);
			return 0;
		}
	}

	//Protect the data!
	size_t protected_buff_len;
	BYTE *protected_buffer;
	const int ret = dpapi_protect_data(&protected_buffer, &protected_buff_len, plaintext_buffer, strlen((const char*)plaintext_buffer) + 1U, salt_buffer, salt_buffer ? strlen((const char*)salt_buffer) : 0U, machine_scope);
	if ((ret <= 0) || (!protected_buffer) || (protected_buff_len <= 0U))
	{
		FREE_TEXT(plaintext_buffer);
		FREE_TEXT(salt_buffer);
		return (ret <= 0) ? ret : 0;
	}

	//Convert result to Base64
	*protected_out = base64_raw2encoded(protected_buffer, protected_buff_len);

	//Clean up
	FREE_TEXT(plaintext_buffer);
	FREE_TEXT(salt_buffer);

	//Return
	return (*protected_out) ? 1 : 0;
}

int dpapi_unprotect_text(TCHAR **const plaintext_out, const TCHAR *const protected_in, const TCHAR *const salt_in)
{
	*plaintext_out = NULL;

	//Check given parameters
	if (!protected_in)
	{
		return 0;
	}

	//Convert Base64 string to "raw" bytes
	size_t protected_buff_len;
	const BYTE *const protected_buffer = base64_encoded2raw(protected_in, protected_buff_len);
	if (!protected_buffer)
	{
		return 0;
	}

	//Convert given salt to bytes, if present
	const BYTE *salt_buffer = NULL;
	if (salt_in && salt_in[0])
	{
		salt_buffer = (const BYTE*)STR_TO_UTF8(salt_in);
		if ((!salt_buffer) || (!salt_buffer[0]))
		{
			free_buffer(protected_buffer, protected_buff_len);
			FREE_TEXT(salt_buffer);
			return 0;
		}
	}

	//Unprotect the data!
	size_t plaintext_buff_len;
	BYTE *plaintext_buffer;
	const int ret = dpapi_unprotect_data(&plaintext_buffer, &plaintext_buff_len, protected_buffer, protected_buff_len, salt_buffer, salt_buffer ? strlen((const char*)salt_buffer) : 0U);
	if ((ret <= 0) || (!plaintext_buffer) || (plaintext_buff_len <= 0U))
	{
		free_buffer(protected_buffer, protected_buff_len);
		FREE_TEXT(salt_buffer);
		return (ret <= 0) ? ret : 0;
	}

	//Convert unprotected bytes to text
	plaintext_buffer[plaintext_buff_len - 1U] = (BYTE)'\0'; /*just to be sure!*/
	*plaintext_out = UTF8_TO_STR((const char*)plaintext_buffer);

	//Clean up
	free_buffer(protected_buffer, protected_buff_len);
	FREE_TEXT(salt_buffer);

	//Return
	return (*plaintext_out) ? 1 : 0;
}

#endif //STDUTILS_TINY_MODE
