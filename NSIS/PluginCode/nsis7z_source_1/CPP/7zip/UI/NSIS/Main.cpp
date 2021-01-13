// Main.cpp

#include "StdAfx.h"

#include "Common/MyInitGuid.h"

#include "Common/CommandLineParser.h"
#include "Common/MyException.h"
#include "Common/IntToString.h"
#include "Common/StdOutStream.h"
#include "Common/StringConvert.h"
#include "Common/StringToInt.h"

#include "Windows/FileDir.h"
#include "Windows/FileName.h"
#include "Windows/Defs.h"
#include "Windows/Error.h"
#ifdef _WIN32
#include "Windows/MemoryLock.h"
#endif

#include "../../IPassword.h"
#include "../../ICoder.h"
#include "../Common/UpdateAction.h"
#include "../Common/Update.h"
#include "../Common/Extract.h"
#include "../Common/ArchiveCommandLine.h"
#include "../Common/ExitCode.h"
#ifdef EXTERNAL_CODECS
#include "../Common/LoadCodecs.h"
#endif

#include "../../Compress/LZMA_Alone/LzmaBenchCon.h"

#include "OpenCallbackConsole.h"
#include "ExtractCallbackConsole.h"

#include "../../MyVersion.h"

#if defined( _WIN32) && defined( _7ZIP_LARGE_PAGES)
extern "C"
{
#include "../../../../C/Alloc.h"
}
#endif

using namespace NWindows;
using namespace NFile;
using namespace NCommandLineParser;

HINSTANCE g_hInstance = 0;

// ---------------------------
// exception messages

#ifndef _WIN32
static void GetArguments(int numArguments, const char *arguments[], UStringVector &parts)
{
  parts.Clear();
  for(int i = 0; i < numArguments; i++)
  {
    UString s = MultiByteToUnicodeString(arguments[i]);
    parts.Add(s);
  }
}
#endif

#ifdef EXTERNAL_CODECS
static void PrintString(CStdOutStream &stdStream, const AString &s, int size)
{
  int len = s.Length();
  stdStream << s;
  for (int i = len; i < size; i++)
    stdStream << ' ';
}
#endif

static inline char GetHex(Byte value)
{
  return (char)((value < 10) ? ('0' + value) : ('A' + (value - 10)));
}

#ifdef _WIN32
void SwitchFileAPIEncoding(BOOL ansi)
{
  if (ansi)
  {
    SetFileApisToOEM();
  }
  else
  {
    SetFileApisToANSI();
  }
}
#endif

int DoExtractArchive(UString archive, UString targetDir, bool overwrite, bool extractPathes, ExtractProgressHandler epc)
{
	CCodecs *codecs = new CCodecs;
	CMyComPtr<IUnknown> compressCodecsInfo = codecs;
	HRESULT result = codecs->Load();

	if (result != S_OK)
		throw CSystemException(result);

	if (codecs->Formats.Size() == 0) throw -1;

	CIntVector formatIndices;

	if (!codecs->FindFormatForArchiveType(L"7z", formatIndices))
	{
		throw -1;
	}

	BOOL bApisAreAnsi = AreFileApisANSI();

#ifdef _WIN32
	if (bApisAreAnsi)
		SwitchFileAPIEncoding(bApisAreAnsi);
#endif

	CExtractCallbackConsole *ecs = new CExtractCallbackConsole;
	CMyComPtr<IFolderArchiveExtractCallback> extractCallback = ecs;
	ecs->ProgressHandler = epc;

#ifndef _NO_CRYPTO
	ecs->PasswordIsDefined = options.PasswordEnabled;
	ecs->Password = options.Password;
#endif

	ecs->Init();

	COpenCallbackConsole openCallback;

#ifndef _NO_CRYPTO
	openCallback.PasswordIsDefined = options.PasswordEnabled;
	openCallback.Password = options.Password;
#endif

	CExtractOptions eo;
	eo.StdOutMode = false;
	eo.PathMode = extractPathes?NExtract::NPathMode::kCurrentPathnames:NExtract::NPathMode::kNoPathnames;
	eo.TestMode = false;
	eo.OverwriteMode = overwrite?NExtract::NOverwriteMode::kWithoutPrompt:NExtract::NOverwriteMode::kSkipExisting;
	eo.OutputDir = targetDir;
	eo.YesToAll = true;
#ifdef COMPRESS_MT
	CObjectVector<CProperty> prp;
	eo.Properties = prp;
#endif
	UString errorMessage;
	CDecompressStat stat;
	NWildcard::CCensor WildcardCensor;
	WildcardCensor.AddItem(true, L"*", true);
	UStringVector ArchivePathsSorted;
	UStringVector ArchivePathsFullSorted;
	ArchivePathsSorted.Add(archive);
	UString fullPath;
	NFile::NDirectory::MyGetFullPathName(archive, fullPath);
	ArchivePathsFullSorted.Add(fullPath);

	result = DecompressArchives(
		codecs,
		formatIndices,
		ArchivePathsSorted,
		ArchivePathsFullSorted,
		WildcardCensor.Pairs.Front().Head,
		eo, &openCallback, ecs, errorMessage, stat);

#ifdef _WIN32
	if (bApisAreAnsi)
		SwitchFileAPIEncoding(!bApisAreAnsi);
#endif


	if (!errorMessage.IsEmpty())
	{
		if (result == S_OK)
		result = E_FAIL;
	}

	if (ecs->NumArchiveErrors != 0 || ecs->NumFileErrors != 0)
	{
		if (result != S_OK)
			throw CSystemException(result);

		return NExitCode::kFatalError;
	}

	if (result != S_OK)
		throw CSystemException(result);

  return 0;
}
