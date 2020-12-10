// isctype.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include <ctype.h>
#include "libct.h"

BEGIN_EXTERN_C

int iswctype(wint_t c, wctype_t type)
{
	WORD ret;
	GetStringTypeW(CT_CTYPE1, (LPCWSTR)&c, 1, &ret);
	if ((ret & type) != 0)
		return 1;
	return 0;
}

//int _ismbcspace(int c)	{return isspace(c);}
int isspace(int c)		{return ((c >= 0x09 && c <= 0x0D) || (c == 0x20));}
int iswspace(wint_t c)	{return iswctype(c, _BLANK);}

//int _ismbcupper(int c)	{return isupper(c);}
int isupper(int c)		{return (c >= 'A' && c <= 'Z');}
int iswupper(wint_t c)	{return iswctype(c, _UPPER);}

//int ismbcalpha(int c)	{return isalpha(c);}
int isalpha(int c)		{return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');}
int iswalpha(wint_t c)	{return iswctype(c, _ALPHA);}

//int ismbcdigit(int c)	{return isdigit(c);}
int isdigit(int c)		{return (c >= '0' && c <= '9');}
int iswdigit(wint_t c)	{return iswctype(c, _DIGIT);}

//int ismbcxdigit(int c)	{return isxdigit(c);}
int isxdigit(int c)		{return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');}
int iswxdigit(wint_t c)	{return iswctype(c, _HEX);}

//int ismbcalnum(int c)	{return isalnum(c);}
int isalnum(int c)		{return isalpha(c) || isdigit(c);}
int iswalnum(wint_t c)	{return iswctype(c, _ALPHA|_DIGIT);}

//int ismbcprint(int c)	{return isprint(c);}
int isprint(int c)		{return c >= ' ';}
int iswprint(wint_t c)	{return iswctype(c, (wctype_t)(~_CONTROL));}

END_EXTERN_C