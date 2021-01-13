// atol.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <stdlib.h>
#include <ctype.h>
#include "libct.h"

BEGIN_EXTERN_C
	
long atol(const char *str)
{
    while (isspace(*str))			// skip whitespace
        ++str;

    int cur = *str++;
    int neg = cur;					// Save the negative sign, if it exists

    if (cur == '-' || cur == '+')
        cur = *str++;

    // While we have digits, add 'em up.

	long total = 0;
    while (isdigit(cur))
    {
        total = 10*total + (cur-'0');			// Add this digit to the total.
        cur = *str++;							// Do the next character.
    }

    // If we have a negative sign, convert the value.
    if (neg == '-')
        return -total;
    else
        return total;
}

int atoi(const char *str)
{
    return (int)atol(str);
}



long wtol(const wchar_t *str)
{
    while (iswspace(*str))			// skip whitespace
        ++str;

    wint_t cur = *str++;
    wint_t neg = cur;					// Save the negative sign, if it exists

    if (cur == L'-' || cur == L'+')
        cur = *str++;

    // While we have digits, add 'em up.

	long total = 0;
    while (iswdigit(cur))
    {
        total = 10*total + (cur-L'0');			// Add this digit to the total.
        cur = *str++;							// Do the next character.
    }

    // If we have a negative sign, convert the value.
    if (neg == L'-')
        return -total;
    else
        return total;
}

int wtoi(const wchar_t *str)
{
    return (int)wtol(str);
}

END_EXTERN_C