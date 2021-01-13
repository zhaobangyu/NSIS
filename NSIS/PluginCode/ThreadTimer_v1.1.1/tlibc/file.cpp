// file.cpp

// file routine overrides

// 08/20/05 (mv)

#include <windows.h>
#include <stdio.h>
#include "libct.h"

/* FILE, as defined in stdio.h
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;				Used to store HANDLE
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
*/

//_flag values (not the ones used by the normal CRT
#define _FILE_TEXT		0x0001
#define _FILE_EOF		0x0002
#define _FILE_ERROR		0x0004

struct _FILE : public FILE
{
	void set_handle(HANDLE h) {_base = (char*)h;};
	HANDLE get_handle() const {return (HANDLE)_base;};
};

// used directly by the stdin, stdout, and stderr macros
_FILE __iob[3];
FILE *__iob_func() {return (FILE*)__iob;}

void _init_file()
{
	// STDIN
	__iob[0].set_handle(GetStdHandle(STD_INPUT_HANDLE));
	__iob[0]._flag = _FILE_TEXT;

	// STDOUT
	__iob[1].set_handle(GetStdHandle(STD_OUTPUT_HANDLE));
	__iob[1]._flag = _FILE_TEXT;

	// STDERR
	__iob[2].set_handle(GetStdHandle(STD_ERROR_HANDLE));
	__iob[2]._flag = _FILE_TEXT;
}


BEGIN_EXTERN_C

/*int _fileno(FILE *fp)
{
	return (int)fp;			// FIXME:  This doesn't work under Win64
}

HANDLE _get_osfhandle(int i)
{
	return (HANDLE)i;		// FIXME:  This doesn't work under Win64
}*/

FILE *fopen(const char *path, const char *attrs)
{
	DWORD access, disp;
	if (strchr(attrs, 'w'))
	{
		access = GENERIC_WRITE;
		disp = CREATE_ALWAYS;
	}
	else
	{
		access = GENERIC_READ;
		disp = OPEN_EXISTING;
	}

	HANDLE hFile = CreateFileA(path, access, 0, 0, disp, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	_FILE *file = new _FILE;
	memset(file, 0, sizeof(_FILE));
	file->set_handle(hFile);

	if (strchr(attrs, 't'))
		file->_flag |= _FILE_TEXT;

	return file;
}

FILE *_wfopen(const wchar_t *path, const wchar_t *attrs)
{
	DWORD access, disp;
	if (wcschr(attrs, L'w'))
	{
		access = GENERIC_WRITE;
		disp = CREATE_ALWAYS;
	}
	else
	{
		access = GENERIC_READ;
		disp = OPEN_EXISTING;
	}

	HANDLE hFile = CreateFileW(path, access, 0, 0, disp, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		return 0;

	_FILE *file = new _FILE;
	memset(file, 0, sizeof(_FILE));
	file->set_handle(hFile);

	if (wcschr(attrs, L't'))
		file->_flag |= _FILE_TEXT;

	return file;
}


int fprintf(FILE *fp, const char *s, ...)
{
	va_list args;
	va_start(args, s);

	char bfr[1024];
	int len = wvsprintfA(bfr, s, args);
	va_end(args);

	fwrite(bfr, len+1, sizeof(char), fp);
	return len;
}

int fwprintf(FILE *fp, const wchar_t *s, ...)
{
	va_list args;
	va_start(args, s);

	wchar_t bfr[1024];
	int len = wvsprintfW(bfr, s, args);

	va_end(args);

	char ansibfr[1024];
	WideCharToMultiByte(CP_ACP, 0, bfr, -1, ansibfr, sizeof(ansibfr), 0, 0);

	fwrite(ansibfr, len+1, sizeof(char), fp);
	return len;
}


int fclose(FILE *fp)
{
	CloseHandle(((_FILE*)fp)->get_handle());
	delete fp;
	return 0;
}

int feof(FILE *fp)
{
	return (fp->_flag & _FILE_EOF) ? 1 : 0;
}

int fseek(FILE *str, long offset, int origin)
{
	DWORD meth = FILE_BEGIN;
	if (origin == SEEK_CUR)
		meth = FILE_CURRENT;
	else if (origin == SEEK_END)
		meth = FILE_END;
	SetFilePointer(((_FILE*)str)->get_handle(), offset, 0, meth);
	((_FILE*)str)->_flag &= ~_FILE_EOF;
	return 0;
}

long ftell(FILE *fp)
{
	return SetFilePointer(((_FILE*)fp)->get_handle(), 0, 0, FILE_CURRENT);
}

size_t fread(void *buffer, size_t size, size_t count, FILE *str)
{
	if (size*count == 0)
		return 0;
	if (feof(str))
		return 0;

	HANDLE hFile = ((_FILE*)str)->get_handle();
	int textMode = ((_FILE*)str)->_flag & _FILE_TEXT;

	char *src;
	if (textMode)
		src = (char*)malloc(size*count);
	else
		src = (char*)buffer;

	DWORD br;
	if (!ReadFile(hFile, src, (DWORD)(size*count), &br, 0))
		((_FILE*)str)->_flag |= _FILE_ERROR;
	else if (!br)		// nonzero return value and no bytes read = EOF
		((_FILE*)str)->_flag |= _FILE_EOF;

	if (!br)
		return 0;

	// Text-mode translation is always ANSI
	if (textMode)		// text mode: must translate CR -> LF
	{
		char *dst = (char*)buffer;
		for (DWORD i = 0; i < br; i++)
		{
			if (src[i] != '\r')
			{
				*dst++ = src[i];
				continue;
			}

			// If next char is LF -> convert CR to LF
			if (i+1 < br)
			{
				if (src[i+1] == '\n')
				{
					*dst++ = '\n';
					i++;
				}
				else
					*dst++ = src[i];
			}
			else if (br > 1)
			{
				// This is the hard part: must peek ahead one byte
				DWORD br2 = 0;
				char peekChar = 0;
				ReadFile(hFile, &peekChar, 1, &br2, 0);
				if (!br2)
					*dst++ = src[i];
				else if (peekChar == '\n')
					*dst++ = '\n';
				else
				{
					fseek(str, -1, SEEK_CUR);
					*dst++ = src[i];
				}
			}
			else
				*dst++ = src[i];
		}

		free(src);
	}

	return br/size;
}

size_t fwrite(const void *buffer, size_t size, size_t count, FILE *str)
{
	DWORD bw = 0, bw2 = 0;

	if (size*count == 0)
		return 0;

	HANDLE hFile = ((_FILE*)str)->get_handle();
	int textMode = ((_FILE*)str)->_flag & _FILE_TEXT;

	// Text-mode translation is always ANSI!
	if (textMode)			// text mode -> translate LF -> CRLF
	{
		const char *src = (const char*)buffer;
		size_t startpos = 0, i = 0;
		for (i = 0; i < size*count; i++)
		{
			if (src[i] != '\n')
				continue;
			if (i > 0 && src[i-1] == '\r')		// don't translate CRLF
				continue;

			if (i > startpos)
			{
				WriteFile(hFile, &src[startpos], i-startpos, &bw2, 0);
				bw += bw2;
			}

			const char *crlf = "\r\n";
			WriteFile(hFile, crlf, 2, &bw2, 0);
			bw++;		// one '\n' written

			startpos = i+1;
		}

		if (i > startpos)
		{
			WriteFile(hFile, &src[startpos], i-startpos, &bw2, 0);
			bw += bw2;
		}
	}
	else
		WriteFile(hFile, buffer, (DWORD)(size*count), &bw, 0);
	return bw/size;
}

char *fgets(char *str, int n, FILE *s)
{
	if (feof(s))
		return 0;

	int i;
	for (i = 0; i < n-1; i++)
	{
		if (!fread(&str[i], 1, sizeof(char), s))
			break;
		if (str[i] == '\r')
		{
			i--;
			continue;
		}
		if (str[i] == '\n')
		{
			i++;
			break;
		}
	}

	str[i] = 0;
	return str;
}

wchar_t *fgetws(wchar_t *str, int n, FILE *s)
{
	// Text-mode fgetws converts MBCS->Unicode
	if (((_FILE*)str)->_flag & _FILE_TEXT)
	{
		char *bfr = (char*)malloc(n);
		fgets(bfr, n, s);
		MultiByteToWideChar(CP_ACP, 0, bfr, -1, str, n);
		free(bfr);
		return str;
	}

	// Binary fgetws reads as Unicode

	if (feof(s))
		return 0;

	int i;
	for (i = 0; i < n-1; i++)
	{
		if (!fread(&str[i], 1, sizeof(wchar_t), s))
			break;
		if (str[i] == L'\r')
		{
			i--;
			continue;	// does i++
		}
		if (str[i] == L'\n')
		{
			i++;
			break;
		}
	}

	str[i] = 0;
	return str;
}


int fgetc(FILE *s)
{
	if (s == 0 || feof(s))
		return EOF;

	char c;
	fread(&c, 1, sizeof(char), s);

	return (int)c;
}

wint_t fgetwc(FILE *s)
{
	if (s == 0 || feof(s))
		return (wint_t)EOF;

	// text-mode fgetwc reads and converts MBCS
	if (((_FILE*)s)->_flag & _FILE_TEXT)
	{
		char ch = (char)fgetc(s);
		wint_t wch;
		MultiByteToWideChar(CP_ACP, 0, &ch, 1, (LPWSTR)&wch, 1);
		return wch;
	}

	// binary fgetwc reads unicode

	wint_t c;
	fread(&c, 1, sizeof(wint_t), s);

	return c;
}


END_EXTERN_C