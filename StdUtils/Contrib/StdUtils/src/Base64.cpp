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

/* Base64 encoder/decoder. Originally Apache file ap_base64.c */

/* ====================================================================
* Copyright (c) 1995-1999 The Apache Group.  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in
*    the documentation and/or other materials provided with the
*    distribution.
*
* 3. All advertising materials mentioning features or use of this
*    software must display the following acknowledgment:
*    "This product includes software developed by the Apache Group
*    for use in the Apache HTTP server project (http://www.apache.org/)."
*
* 4. The names "Apache Server" and "Apache Group" must not be used to
*    endorse or promote products derived from this software without
*    prior written permission. For written permission, please contact
*    apache@apache.org.
*
* 5. Products derived from this software may not be called "Apache"
*    nor may "Apache" appear in their names without prior written
*    permission of the Apache Group.
*
* 6. Redistributions of any form whatsoever must retain the following
*    acknowledgment:
*    "This product includes software developed by the Apache Group
*    for use in the Apache HTTP server project (http://www.apache.org/)."
*
* THIS SOFTWARE IS PROVIDED BY THE APACHE GROUP ``AS IS'' AND ANY
* EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE APACHE GROUP OR
* ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
* ====================================================================
*
* This software consists of voluntary contributions made by many
* individuals on behalf of the Apache Group and was originally based
* on public domain software written at the National Center for
* Supercomputing Applications, University of Illinois, Urbana-Champaign.
* For more information on the Apache Group and the Apache HTTP server
* project, please see <http://www.apache.org/>.
*
*/

#include "Base64.h"
#include "UnicodeSupport.h"

#include <string.h>
#include <malloc.h>

/* aaaack but it's fast and const should make it shared text page. */
static const BYTE pr2six[256] =
{
	/* ASCII table */
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
	64, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
	64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
};

static const TCHAR basis_64[] = T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

size_t base64_decode_len(const TCHAR *const bufcoded)
{
	size_t nbytesdecoded;
	register const TBYTE *bufin;
	register size_t nprbytes;

	bufin = (const TBYTE*)bufcoded;
	while (pr2six[*(bufin++)] <= 63);

	nprbytes = (bufin - (const TBYTE*)bufcoded) - 1;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;

	return nbytesdecoded + 1;
}

size_t base64_decode(BYTE *const bufplain, const TCHAR *const bufcoded)
{
	size_t nbytesdecoded;
	register const TBYTE *bufin;
	register BYTE *bufout;
	register size_t nprbytes;

	bufin = (const TBYTE*)bufcoded;
	while (pr2six[*(bufin++)] <= 63);
	nprbytes = (bufin - (const TBYTE*)bufcoded) - 1;
	nbytesdecoded = ((nprbytes + 3) / 4) * 3;

	bufout = bufplain;
	bufin = (const TBYTE*)bufcoded;

	while (nprbytes > 4)
	{
		*(bufout++) = (BYTE)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
		*(bufout++) = (BYTE)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
		*(bufout++) = (BYTE)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
		bufin += 4;
		nprbytes -= 4;
	}

	/* Note: (nprbytes == 1) would be an error, so just ingore that case */
	if (nprbytes > 1)
	{
		*(bufout++) = (BYTE)(pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
	}
	if (nprbytes > 2)
	{
		*(bufout++) = (BYTE)(pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
	}
	if (nprbytes > 3)
	{
		*(bufout++) = (BYTE)(pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
	}

	*(bufout++) = '\0';
	nbytesdecoded -= (4 - nprbytes) & 3;
	return nbytesdecoded;
}

size_t base64_encode_len(size_t len)
{
	return ((len + 2) / 3 * 4) + 1;
}

size_t base64_encode(TCHAR *const encoded, const BYTE *const data, const size_t len)
{
	size_t i;
	TCHAR *p;

	p = encoded;
	for (i = 0; i + 2 < len; i += 3)
	{
		*p++ = basis_64[(data[i] >> 2) & 0x3F];
		*p++ = basis_64[((data[i] & 0x3) << 4) | ((size_t)(data[i + 1] & 0xF0) >> 4)];
		*p++ = basis_64[((data[i + 1] & 0xF) << 2) | ((size_t)(data[i + 2] & 0xC0) >> 6)];
		*p++ = basis_64[data[i + 2] & 0x3F];
	}
	if (i < len)
	{
		*p++ = basis_64[(data[i] >> 2) & 0x3F];
		if (i == (len - 1))
		{
			*p++ = basis_64[((data[i] & 0x3) << 4)];
			*p++ = '=';
		}
		else
		{
			*p++ = basis_64[((data[i] & 0x3) << 4) | ((size_t)(data[i + 1] & 0xF0) >> 4)];
			*p++ = basis_64[((data[i + 1] & 0xF) << 2)];
		}
		*p++ = '=';
	}

	*p++ = '\0';
	return p - encoded;
}

TCHAR *base64_raw2encoded(const BYTE *const bufplain, const size_t len)
{
	const size_t encoded_len = base64_encode_len(len);
	if (encoded_len < 1U)
	{
		return NULL;
	}

	TCHAR *const encoded_buffer = new TCHAR[encoded_len];
	if (base64_encode(encoded_buffer, bufplain, len) < 1U)
	{
		memset(encoded_buffer, 0, encoded_len * sizeof(TCHAR));
		delete [] encoded_buffer;
		return NULL;
	}

	return encoded_buffer;
}

BYTE *base64_encoded2raw(const TCHAR *const encoded, size_t &data_len)
{
	data_len = 0U;

	const size_t decoded_len = base64_decode_len(encoded);
	if (decoded_len < 1U)
	{
		return NULL;
	}

	BYTE *const decoded_buffer = new BYTE[decoded_len];
	if ((data_len = base64_decode(decoded_buffer, encoded)) < 1U)
	{
		memset(decoded_buffer, 0, decoded_len * sizeof(BYTE));
		delete [] decoded_buffer;
		return NULL;
	}

	return decoded_buffer;
}