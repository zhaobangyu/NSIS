#include <windows.h>
#include <process.h>
#include <string.h>
#ifndef UNICODE
#include <nsis_ansi/pluginapi.h>
#else
#include <nsis_unicode/pluginapi.h>
#endif

typedef struct _THREADPARAM {
	int iInterval;
	int iDelay;
	int iFunction;
	extra_parameters *extra;
} THREADPARAM, *PTHREADPARAM;

static HINSTANCE g_hInstance;
static THREADPARAM g_ThreadParam;
static BOOLEAN g_bStopImmediately;

static DWORD WINAPI ThreadProc(void *);
static UINT_PTR PluginCallback(enum NSPIM);

__declspec(dllexport) void Start(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	int iTemp;
	DWORD uThreadID;
	HANDLE hThread;
	EXDLL_INIT();
	extra->RegisterPluginCallback(g_hInstance, PluginCallback);
	iTemp = popint();
	if (iTemp <= 0) {
		extra->exec_flags->exec_error = TRUE;
		pushint(1);
		return;
	}
	g_ThreadParam.iInterval = iTemp;
	g_ThreadParam.iDelay = popint();
	iTemp = popint();
	if (iTemp <= 0) {
		extra->exec_flags->exec_error = TRUE;
		pushint(3);
		return;
	}
	g_ThreadParam.iFunction = iTemp;
	g_ThreadParam.extra = extra;
	g_bStopImmediately = FALSE;

	hThread = (HANDLE)CreateThread(NULL, 0, ThreadProc, &g_ThreadParam, 0, &uThreadID);
	CloseHandle(hThread);
}

__declspec(dllexport) void Stop(HWND hwndParent, int string_size, TCHAR *variables, stack_t **stacktop, extra_parameters *extra)
{
	g_bStopImmediately = TRUE;
}

static DWORD WINAPI ThreadProc(void *lParam)
{
	int iTime;
	PTHREADPARAM pThreadParam;
	pThreadParam = (PTHREADPARAM)lParam;
	if (pThreadParam->iDelay > 0) {
		for (iTime = 0; iTime < pThreadParam->iDelay; iTime++) {
			if (g_bStopImmediately) {
				break;
			}
			Sleep(pThreadParam->iInterval);
			if (g_bStopImmediately) {
				break;
			}
			pThreadParam->extra->ExecuteCodeSegment(pThreadParam->iFunction - 1, 0);
		}
	} else {
		while (!g_bStopImmediately) {
			Sleep(pThreadParam->iInterval);
			pThreadParam->extra->ExecuteCodeSegment(pThreadParam->iFunction - 1, 0);
		}
	}
	return 0;
}

static UINT_PTR PluginCallback(enum NSPIM msg)
{
	return 0;
}

BOOL WINAPI DllMain(HANDLE hInstance, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	g_hInstance = (HINSTANCE)hInstance;
	return TRUE;
}
