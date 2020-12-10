#include "StdAfx.h"

#include <windows.h>
#include <commctrl.h>

#include "NSISPluginSample\pluginapi.h" // nsis plugin
#include "../../UI/NSIS/ExtractCallbackConsole.h"

#define IDC_PROGRESS                    1004
#define IDC_INTROTEXT                   1006

#define EXTRACTFUNC(funcname) extern "C" { \
void  __declspec(dllexport) __cdecl funcname(HWND hwndParent, int string_size, \
                                      char *variables, stack_t **stacktop, \
                                      extra_parameters *extra) \
{ \
	EXDLL_INIT();\
	lastVal = -1; \
	g_hwndParent=hwndParent; \
	HWND hwndDlg = FindWindowEx(g_hwndParent, NULL, "#32770", NULL); \
	g_hwndProgress = GetDlgItem(hwndDlg, IDC_PROGRESS); \
	g_hwndText = GetDlgItem(hwndDlg, IDC_INTROTEXT); \
	char sArchive[1024], *outDir = getuservariable(INST_OUTDIR); \
	popstring(sArchive); \

#define EXTRACTFUNCEND } }

HINSTANCE g_hInstance2;
HWND g_hwndParent;
HWND g_hwndProgress;
HWND g_hwndText;
extra_parameters *g_pluginExtra;

void DoInitialize();
int DoExtract(LPSTR archive, LPSTR dir, bool overwrite, bool expath, ExtractProgressHandler epc);

int progressCallback = -1;
UInt64 lastVal = -1;
char sDetails[1024];
void SimpleProgressHandler(UInt64 completedSize, UInt64 totalSize)
{
	UInt64 val = 0, nsisProgressMax = 30000;
	if (completedSize > 0 || totalSize != 0)
	{
		val = (completedSize*nsisProgressMax)/totalSize;
		if (val < 0) val = 0;
		if (val > nsisProgressMax) val = nsisProgressMax;
	}
	if (lastVal != val)
		SendMessage(g_hwndProgress,PBM_SETPOS,(int)(lastVal = val),0);
}

void CallbackProgressHandler(UInt64 completedSize, UInt64 totalSize)
{
	UInt64 val = 0, nsisProgressMax = 30000;
	if (completedSize > 0 || totalSize != 0)
	{
		val = (completedSize*nsisProgressMax)/totalSize;
		if (val < 0) val = 0;
		if (val > nsisProgressMax) val = nsisProgressMax;

		pushint((int)totalSize);
		pushint((int)completedSize);
		g_pluginExtra->ExecuteCodeSegment(progressCallback-1, 0);
	}
	if (lastVal != val)
		SendMessage(g_hwndProgress,PBM_SETPOS,(int)(lastVal = val),0);
}

void DetailsProgressHandler(UInt64 completedSize, UInt64 totalSize)
{
	UInt64 val = 0, nsisProgressMax = 30000;
	if (completedSize > 0 || totalSize != 0)
	{
		val = (completedSize*nsisProgressMax)/totalSize;
		if (val < 0) val = 0;
		if (val > nsisProgressMax) val = nsisProgressMax;
		char buf[1024], buf2[1024];
		wsprintf(buf, "%d%% (%d / %d MB)", (int) (val/300), (int)(completedSize/(1024*1024)), (int)(totalSize/(1024*1024)));
		wsprintf(buf2, sDetails, buf);
		SetWindowText(g_hwndText, buf2);
	}
	if (lastVal != val)
		SendMessage(g_hwndProgress,PBM_SETPOS,(int)(lastVal = val),0);
}

EXTRACTFUNC(Extract)
{
	DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)SimpleProgressHandler);
}
EXTRACTFUNCEND

EXTRACTFUNC(ExtractWithDetails)
{
	popstring(sDetails);
	DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)DetailsProgressHandler);
}
EXTRACTFUNCEND

EXTRACTFUNC(ExtractWithCallback)
{
	progressCallback = popint();
	DoExtract(sArchive, outDir, true, true, (ExtractProgressHandler)CallbackProgressHandler);
}
EXTRACTFUNCEND

extern "C" BOOL WINAPI DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance2=(HINSTANCE)hInst;
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		DoInitialize();
	}
	return TRUE;
}
