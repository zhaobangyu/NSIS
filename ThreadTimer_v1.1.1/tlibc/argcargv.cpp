// argcargv.cpp

// based on:
// LIBCTINY - Matt Pietrek 2001
// MSDN Magazine, January 2001

// 08/12/06 (mv)

#include <windows.h>
#include "libct.h"

#define _MAX_CMD_LINE_ARGS  32
TCHAR *_argv[_MAX_CMD_LINE_ARGS+1];
static TCHAR *_rawCmd = 0;

int _init_args()
{
	_argv[0] = 0;

	TCHAR *sysCmd = GetCommandLine();
	int szSysCmd = lstrlen(sysCmd);

	// copy the system command line
	TCHAR *cmd = (TCHAR*)HeapAlloc(GetProcessHeap(), 0, sizeof(TCHAR)*(szSysCmd+1));
	_rawCmd = cmd;
	if (!cmd)
		return 0;
	lstrcpy(cmd, sysCmd);

	// Handle a quoted filename
	if (*cmd == _T('"'))
	{
		cmd++;
		_argv[0] = cmd;						// argv[0] = exe name

		while (*cmd && *cmd != _T('"'))
			cmd++;

		if (*cmd)
			*cmd++ = 0;
		else
			return 0;						// no end quote!
	}
	else
	{
		_argv[0] = cmd;						// argv[0] = exe name

		while (*cmd && !_istspace(*cmd))
			cmd++;

		if (*cmd)
			*cmd++ = 0;
	}

	int argc = 1;
	for (;;)
	{
		while (*cmd && _istspace(*cmd))		// Skip over any whitespace
			cmd++;

		if (*cmd == 0)						// End of command line?
			return argc;

		if (*cmd == _T('"'))					// Argument starting with a quote???
		{
			cmd++;

			_argv[argc++] = cmd;
			_argv[argc] = 0;

			while (*cmd && *cmd != _T('"'))
				cmd++;

			if (*cmd == 0)
				return argc;

			if (*cmd)
				*cmd++ = 0;
		}
		else
		{
			_argv[argc++] = cmd;
			_argv[argc] = 0;

			while (*cmd && !_istspace(*cmd))
				cmd++;

			if (*cmd == 0)
				return argc;

			if (*cmd)
				*cmd++ = 0;
		}

		if (argc >= _MAX_CMD_LINE_ARGS)
			return argc;
	}
}

void _term_args()
{
	if (_rawCmd)
		HeapFree(GetProcessHeap(), 0, _rawCmd);
}