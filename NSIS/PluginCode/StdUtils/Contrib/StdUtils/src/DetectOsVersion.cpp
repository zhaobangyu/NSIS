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

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

#include <climits>

#include "UnicodeSupport.h"
#include "DetectOsVersion.h"
#include "Mutex.h"
#include "msvc_utils.h"

//Forward declaration
static bool get_os_info(OSVERSIONINFOEXW *const osInfo);
static bool verify_os_version(const DWORD major, const DWORD minor, const WORD spack);
static bool verify_os_buildNo(const DWORD buildNo);
static inline DWORD initialize_step_size(const DWORD &limit);
static void sanitize_nt_headers(const HMODULE hModule);

//External vars
extern bool g_bStdUtilsVerbose;
extern RTL_CRITICAL_SECTION g_pStdUtilsMutex;
extern HINSTANCE g_StdUtilsInstance;

//Lazy initialization vars
static volatile unsigned int g_os_version_major = 0;
static volatile unsigned int g_os_version_minor = 0;
static volatile unsigned int g_os_version_spack = 0;
static volatile unsigned int g_os_version_build = 0;
static volatile bool g_os_version_bOverride = false;

/*
 * Add with overflow protection
 */
static inline DWORD SAFE_ADD(const DWORD &a, const DWORD &b, const DWORD &limit = MAXDWORD)
{
	return ((a >= limit) || (b >= limit) || ((limit - a) <= b)) ? limit : (a + b);
}

/*
 * Determine the *real* Windows version
 */
bool get_real_os_version(unsigned int *const major, unsigned int *const minor, unsigned int *const spack, bool *const pbOverride)
{
	static const DWORD MAX_VALUE = 0xFFFF;

	*major = *minor = *spack = 0;
	*pbOverride = false;
	
	//Enter critical section
	MutexLocker lock(&g_pStdUtilsMutex);

	//Already initialized?
	if((g_os_version_major != 0) || (g_os_version_minor != 0) || (g_os_version_spack != 0))
	{
		*major = g_os_version_major;
		*minor = g_os_version_minor;
		*spack = g_os_version_spack;
		*pbOverride = g_os_version_bOverride;
		return true;
	}

	//Temporary unlock
	lock.unlock();

	//Let's do some basic sanity checking first
	sanitize_nt_headers(g_StdUtilsInstance);
	sanitize_nt_headers(GetModuleHandle(NULL));

	//Initialize local variables
	OSVERSIONINFOEXW osvi;

	//Try GetVersionEx() first
	if(get_os_info(&osvi) == FALSE)
	{
		if(g_bStdUtilsVerbose)
		{
			MessageBox(0, T("GetVersionEx() has failed, cannot detect Windows version!"), T("StdUtils::get_real_os_version"), MB_ICONERROR|MB_TOPMOST);
		}
		return false;
	}

	//Make sure we are running on NT
	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		*major = osvi.dwMajorVersion;
		*minor = osvi.dwMinorVersion;
		*spack = osvi.wServicePackMajor;
	}
	else
	{
		//Workaround for Windows 9x comaptibility mode
		if(verify_os_version(4, 0, 0))
		{
			*pbOverride = true;
			*major = 4;
		}
		else
		{
			//Really not running on Windows NT
			return false;
		}
	}

	//Determine the real *major* version first
	for(DWORD nextMajor = (*major) + 1; nextMajor <= MAX_VALUE; nextMajor++)
	{
		if(verify_os_version(nextMajor, 0, 0))
		{
			*major = nextMajor;
			*minor = *spack = 0;
			*pbOverride = true;
			continue;
		}
		break;
	}

	//Now also determine the real *minor* version
	for(DWORD nextMinor = (*minor) + 1; nextMinor <= MAX_VALUE; nextMinor++)
	{
		if(verify_os_version((*major), nextMinor, 0))
		{
			*minor = nextMinor;
			*spack = 0;
			*pbOverride = true;
			continue;
		}
		break;
	}

	//Finally determine the real *servicepack* version
	for(WORD nextSpack = (*spack) + 1; nextSpack <= MAX_VALUE; nextSpack++)
	{
		if(verify_os_version((*major), (*minor), nextSpack))
		{
			*spack = nextSpack;
			*pbOverride = true;
			continue;
		}
		break;
	}

	//Overflow detected?
	if((*major >= MAX_VALUE) || (*minor >= MAX_VALUE) || (*spack >= MAX_VALUE))
	{
		return false;
	}

	//Enter critical section
	lock.relock();

	//Save the results
	if((g_os_version_major == 0) && (g_os_version_minor == 0) && (g_os_version_spack == 0))
	{
		g_os_version_major = *major;
		g_os_version_minor = *minor;
		g_os_version_spack = *spack;
		g_os_version_bOverride = g_os_version_bOverride || (*pbOverride);
	}
	else
	{
		*major = g_os_version_major;
		*minor = g_os_version_minor;
		*spack = g_os_version_spack;
		*pbOverride = g_os_version_bOverride;
	}

	//Done
	return true;
}

/*
 * Determine the *real* Windows build number
 */
bool get_real_os_buildNo(unsigned int *const buildNo, bool *const pbOverride)
{
	static const DWORD MAX_BUILDNO = (((DWORD)~((DWORD)0)) >> 1);

	*buildNo = 0;
	*pbOverride = false;
	
	//Enter critical section
	MutexLocker lock(&g_pStdUtilsMutex);

	//Already initialized?
	if(g_os_version_build != 0)
	{
		*buildNo = g_os_version_build;
		*pbOverride = g_os_version_bOverride;
		return true;
	}

	//Temporary unlock
	lock.unlock();

	//Initialize local variables
	OSVERSIONINFOEXW osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

	//Try GetVersionEx() first
	if(get_os_info(&osvi) == FALSE)
	{
		if(g_bStdUtilsVerbose)
		{
			MessageBox(0, T("GetVersionEx() has failed, cannot detect Windows version!"), T("StdUtils::get_real_os_buildNo"), MB_ICONERROR|MB_TOPMOST);
		}
		return false;
	}

	//Make sure we are running on NT
	if(osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		*buildNo = osvi.dwBuildNumber;
	}
	else
	{
		//Workaround for Windows 9x comaptibility mode
		if(verify_os_version(4, 0, 0))
		{
			*pbOverride = true;
			*buildNo = 1381;
		}
		else
		{
			//Really not running on Windows NT
			return false;
		}
	}

	//Determine the real build number
	if (verify_os_buildNo(SAFE_ADD((*buildNo), 1, MAX_BUILDNO)))
	{
		DWORD stepSize = initialize_step_size(MAX_BUILDNO);
		for (DWORD nextBuildNo = SAFE_ADD((*buildNo), stepSize, MAX_BUILDNO); (*buildNo) < MAX_BUILDNO; nextBuildNo = SAFE_ADD((*buildNo), stepSize, MAX_BUILDNO))
		{
			if (verify_os_buildNo(nextBuildNo))
			{
				*buildNo = nextBuildNo;
				*pbOverride = true;
				continue;
			}
			if (stepSize > 1)
			{
				stepSize = stepSize / 2;
				continue;
			}
			break;
		}
	}

	//Enter critical section
	lock.relock();

	//Save the results
	if(g_os_version_build == 0)
	{
		g_os_version_build = *buildNo;
		g_os_version_bOverride = g_os_version_bOverride || (*pbOverride);
	}
	else
	{
		*buildNo = g_os_version_build;
		*pbOverride = g_os_version_bOverride;
	}

	//Done
	return true;
}

/*
 * Get friendly OS version name
 */
const TCHAR *get_os_friendly_name(const unsigned int major, const unsigned int minor, const bool server)
{
	static const size_t NAME_COUNT = 9;
	static const struct
	{
		const DWORD major;
		const DWORD minor;
		const TCHAR name_ws[32];
		const TCHAR name_sv[32];
	}
	s_names[NAME_COUNT] =
	{
		{  4, 0, T("Windows NT 4.0"  ), T("Windows NT 4.0 (Server)") },
		{  5, 0, T("Windows 2000"    ), T("Windows 2000 (Server)"  ) },
		{  5, 1, T("Windows XP"      ), T("Windows XP (Server)"    ) },
		{  5, 2, T("Windows XP (x64)"), T("Windows Server 2003"    ) },
		{  6, 0, T("Windows Vista"   ), T("Windows Server 2008"    ) },
		{  6, 1, T("Windows 7"       ), T("Windows Server 2008 R2" ) },
		{  6, 2, T("Windows 8"       ), T("Windows Server 2012"    ) },
		{  6, 3, T("Windows 8.1"     ), T("Windows Server 2012 R2" ) },
		{ 10, 0, T("Windows 10"      ), T("Windows Server 2016"    ) }
	};

	for(size_t i = 0; i < NAME_COUNT; i++)
	{
		if((s_names[i].major == major) && (s_names[i].minor == minor))
		{
			return server ? (&s_names[i].name_sv[0]) : (&s_names[i].name_ws[0]);
		}
	}

	if(g_bStdUtilsVerbose)
	{
		TCHAR buffer[256];
		SNPRINTF(buffer, 256, T("Running on an unknown windows version v%u.%u!"), major, minor);
		buffer[255] = '\0';
		MessageBox(0, buffer, T("StdUtils::get_os_friendly_name"), MB_ICONWARNING|MB_TOPMOST);
	}

	return T("unknown");
}

/*
 * Checks whether OS is a "server" (or "workstation") edition
 */
bool get_os_server_edition(bool *const bIsServer)
{
	bool success = false;

	//Initialize local variables
	OSVERSIONINFOEXW osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

	//Check for server/workstation edition
	if(get_os_info(&osvi))
	{
		switch(osvi.wProductType)
		{
		case VER_NT_SERVER:
		case VER_NT_DOMAIN_CONTROLLER:
			success = true;
			*bIsServer = true;
			break;
		case VER_NT_WORKSTATION:
			success = true;
			*bIsServer = false;
			break;
		}
	}

	return success;
}

/*
 * Get the Windows 10 release ID
 */
int get_os_release_id(DWORD *const releaseId)
{
	static const size_t RELEASE_COUNT = 7;
	static const struct
	{
		const DWORD buildNmbr;
		const DWORD releaseId;
	}
	s_releases[RELEASE_COUNT] =
	{
		{ 10240, 1507 }, //Threshold 1
		{ 10586, 1511 }, //Threshold 2
		{ 14393, 1607 }, //Redstone 1
		{ 15063, 1703 }, //Redstone 2
		{ 16299, 1709 }, //Redstone 3
		{ 17134, 1803 }, //Redstone 4
		{ 17763, 1809 }  //Redstone 5
	};

	*releaseId = DWORD(-1);

	//Check for Windows 10
	unsigned int realOsMajor, realOsMinor, realOsSPack;
	bool overrideFlag;
	if(get_real_os_version(&realOsMajor, &realOsMinor, &realOsSPack, &overrideFlag))
	{
		if(realOsMajor < 10)
		{
			return 0; /*Not running on Windows 10+*/
		}
	}
	else
	{
		return -1; /*failed to detect real OS version*/
	}

	//Read the real OS build number
	unsigned int realOsBuild;
	if(get_real_os_buildNo(&realOsBuild, &overrideFlag))
	{
		//Try to get release id from LUT
		for(size_t i = 0; i < RELEASE_COUNT; ++i)
		{
			if(realOsBuild == s_releases[i].buildNmbr)
			{
				*releaseId = s_releases[i].releaseId;
				return 1;
			}
		}
	}

	//Read the release id from registry
	HKEY hKeyCurrentVersion;
	if(RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE, &hKeyCurrentVersion) == ERROR_SUCCESS)
	{
		const DWORD buffSize = 256;
		BYTE data[buffSize];
		DWORD type = REG_NONE, size = buffSize;
		if(RegQueryValueExW(hKeyCurrentVersion, L"ReleaseId", NULL, &type, &data[0], &size) == ERROR_SUCCESS)
		{
			if(((type == REG_SZ) || (type == REG_EXPAND_SZ) || (type == REG_MULTI_SZ)) && (size >= sizeof(WCHAR)))
			{
				if(const WCHAR *const regIdStr = reinterpret_cast<const WCHAR*>(&data[0]))
				{
					unsigned long regIdVal;
					if(swscanf(regIdStr, L"%lu", &regIdVal) == 1)
					{
						*releaseId = regIdVal;
					}
				}
			}
			else if((type == REG_DWORD) && (size >= sizeof(DWORD)))
			{
				if(const DWORD *const regIdPtr = reinterpret_cast<const DWORD*>(&data[0]))
				{
					*releaseId = (*regIdPtr);
				}
			}
			else if((type == REG_QWORD) && (size >= sizeof(ULARGE_INTEGER)))
			{
				if(const ULARGE_INTEGER *const regIdPtr = reinterpret_cast<const ULARGE_INTEGER*>(&data[0]))
				{
					*releaseId = regIdPtr->LowPart;
				}
			}
		}
		RegCloseKey(hKeyCurrentVersion);
	}

	return ((*releaseId) != DWORD(-1)) ? 1 : (-1);
}

/*
 * Get friendly name of the Windows 10 release ID
 */
const TCHAR *get_os_release_name(const unsigned int releaseId)
{
	static const size_t NAME_COUNT = 8;
	static const struct
	{
		const DWORD releaseId;
		const TCHAR releaseName[32];
	}
	s_names[NAME_COUNT] =
	{
		{ 1507 , T("Threshold 1") },
		{ 1511 , T("Threshold 2") },
		{ 1607 , T("Redstone 1" ) },
		{ 1703 , T("Redstone 2" ) },
		{ 1709 , T("Redstone 3" ) },
		{ 1803 , T("Redstone 4" ) },
		{ 1809 , T("Redstone 5" ) },
		{ 1903 , T("19H1"       ) }
	};

	for(size_t i = 0; i < NAME_COUNT; i++)
	{
		if(s_names[i].releaseId == releaseId)
		{
			return s_names[i].releaseName;
		}
	}

	return T("unknown");
}

//===========================================================================
// INTERNAL FUNCTIONS
//===========================================================================

typedef LONG(__stdcall *RtlGetVersion)(LPOSVERSIONINFOEXW);
typedef LONG(__stdcall *RtlVerifyVersionInfo)(LPOSVERSIONINFOEXW, ULONG, ULONGLONG);

/*
 * Initialize OSVERSIONINFOEXW structure
 */
static void initialize_os_version(OSVERSIONINFOEXW *const osInfo)
{
	memset(osInfo, 0, sizeof(OSVERSIONINFOEXW));
	osInfo->dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
}

/*
 * Determine initial step size
 */
static inline DWORD initialize_step_size(const DWORD &limit)
{
	DWORD result = 1;
	while (result < limit)
	{
		result = SAFE_ADD(result, result);
	}
	return result;
}

/*
 * Get OS version info
 */
static bool get_os_info(OSVERSIONINFOEXW *const osInfo)
{
	if (const HMODULE ntdll = GetModuleHandleW(L"ntdll"))
	{
		if (const RtlGetVersion pRtlGetVersion = (RtlGetVersion) GetProcAddress(ntdll, "RtlGetVersion"))
		{
			initialize_os_version(osInfo);
			if (pRtlGetVersion(osInfo) == 0)
			{
				return true;
			}
		}
	}

	//Fallback
	initialize_os_version(osInfo);
	return (GetVersionExW((LPOSVERSIONINFOW)osInfo) != FALSE);
}

/*
 * Verify OS version info
 */
static bool verify_os_info(OSVERSIONINFOEXW *const osInfo, const ULONG typeMask, const ULONGLONG condMask)
{
	if (const HMODULE ntdll = GetModuleHandleW(L"ntdll"))
	{
		if (const RtlVerifyVersionInfo pRtlVerifyVersionInfo = (RtlVerifyVersionInfo) GetProcAddress(ntdll, "RtlVerifyVersionInfo"))
		{
			if (pRtlVerifyVersionInfo(osInfo, typeMask, condMask) == 0)
			{
				return true;
			}
		}
	}

	//Fallback
	return (VerifyVersionInfoW(osInfo, typeMask, condMask) != FALSE);
}

/*
 * Verify a specific Windows version
 */
static bool verify_os_version(const DWORD major, const DWORD minor, const WORD spack)
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG dwlConditionMask = 0;

	//Initialize the OSVERSIONINFOEX structure
	initialize_os_version(&osvi);

	//Fille the OSVERSIONINFOEX structure
	osvi.dwMajorVersion    = major;
	osvi.dwMinorVersion    = minor;
	osvi.wServicePackMajor = spack;
	osvi.dwPlatformId      = VER_PLATFORM_WIN32_NT;

	//Initialize the condition mask
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION,     VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION,     VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_PLATFORMID,       VER_EQUAL);

	// Perform the test
	const BOOL ret = verify_os_info(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR | VER_PLATFORMID, dwlConditionMask);

	//Error checking
	if(!ret)
	{
		if(GetLastError() != ERROR_OLD_WIN_VERSION)
		{
			if(g_bStdUtilsVerbose)
			{
				MessageBox(0, T("VerifyVersionInfo() has failed, cannot test Windows version!"), T("StdUtils::verify_os_version"), MB_ICONERROR|MB_TOPMOST);
			}
		}
	}

	return (ret != FALSE);
}

/*
 * Verify a specific Windows build
 */
static bool verify_os_buildNo(const DWORD buildNo)
{
	OSVERSIONINFOEXW osvi;
	DWORDLONG dwlConditionMask = 0;

	//Initialize the OSVERSIONINFOEX structure
	initialize_os_version(&osvi);

	//Fille the OSVERSIONINFOEX structure
	osvi.dwBuildNumber = buildNo;

	//Initialize the condition mask
	VER_SET_CONDITION(dwlConditionMask, VER_BUILDNUMBER, VER_GREATER_EQUAL);

	// Perform the test
	const BOOL ret = verify_os_info(&osvi, VER_BUILDNUMBER, dwlConditionMask);

	//Error checking
	if(!ret)
	{
		if(GetLastError() != ERROR_OLD_WIN_VERSION)
		{
			if(g_bStdUtilsVerbose)
			{
				MessageBox(0, T("VerifyVersionInfo() has failed, cannot test Windows version!"), T("StdUtils::verify_os_buildNo"), MB_ICONERROR|MB_TOPMOST);
			}
		}
	}

	return (ret != FALSE);
}

/*
 * Detect EXE and DLL header corruptions
 */
void sanitize_nt_headers(const HMODULE hModule)
{
	if (hModule)
	{
		const BYTE *const baseAddr = reinterpret_cast<const BYTE*>(hModule);
		if (reinterpret_cast<const IMAGE_NT_HEADERS32*>(reinterpret_cast<const IMAGE_DOS_HEADER*>(baseAddr)->e_lfanew + baseAddr)->OptionalHeader.Win32VersionValue)
		{
			TCHAR exeFileName[MAX_PATH], messageBuff[MAX_PATH + 40];
			const DWORD ret = GetModuleFileName(hModule, exeFileName, MAX_PATH);
			if((ret > 0) && (ret < MAX_PATH))
			{
				if(SNPRINTF(messageBuff, MAX_PATH + 40, T("%s is not a valid Win32 application!"), exeFileName) > 0)
				{
					FatalAppExit(0, messageBuff);
					TerminateProcess(GetCurrentProcess(), UINT(-1));
				}
			}
			FatalAppExit(0, T("This is not a valid Win32 application!"));
			TerminateProcess(GetCurrentProcess(), UINT(-1));
		}
	}
}

/*eof*/
