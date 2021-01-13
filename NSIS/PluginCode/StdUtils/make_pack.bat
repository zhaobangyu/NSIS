@echo off
setlocal enabledelayedexpansion
REM -------------------------------------------------------------------------
set "MSC_PATH=c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\"
set "GIT_PATH=c:\Program Files\Git"
REM -------------------------------------------------------------------------
set "PATH=%GIT_PATH%;%GIT_PATH%\mingw64\bin;%GIT_PATH%\usr\bin;%PATH%"
call "%MSC_PATH%\vcvarsall.bat" x86
if "%VCINSTALLDIR%"=="" (
	pause
	exit
)
REM -------------------------------------------------------------------------
set "ISO_DATE="
if not exist "%~dp0\Contrib\StdUtils\utils\Date.exe" GOTO:EOF
for /F "tokens=1,2 delims=:" %%a in ('"%~dp0\Contrib\StdUtils\utils\Date.exe" +ISODATE:%%Y-%%m-%%d') do (
	if "%%a"=="ISODATE" set "ISO_DATE=%%b"
)
if "%ISO_DATE%"=="" (
	pause
	exit
)
REM -------------------------------------------------------------------------
if exist "%~dp0\StdUtils.%ISO_DATE%.zip" (
	attrib -r "%~dp0\StdUtils.%ISO_DATE%.zip"
	del "%~dp0\StdUtils.%ISO_DATE%.zip"
)
if exist "%~dp0\StdUtils.%ISO_DATE%.zip" (
	pause
	exit
)
REM -------------------------------------------------------------------------
set "CONFIG_NAMES=ANSI,Unicode,Tiny"
for %%c in (%CONFIG_NAMES%) do (
	for %%t in (Clean,Rebuild,Build) do (
		MSBuild.exe /property:Configuration=Release_%%c /property:Platform=Win32 /target:%%t /verbosity:normal "%~dp0\Contrib\StdUtils\StdUtils.sln"
		if not "!ERRORLEVEL!"=="0" (
			pause
			exit
		)
	)
)
call "%~dp0\Docs\StdUtils\minify.cmd"
REM -------------------------------------------------------------------------
set "PACK_PATH=%TMP%\~%RANDOM%%RANDOM%.tmp"
mkdir "%PACK_PATH%"
for %%k in (Plugins,Include) do (
	mkdir "%PACK_PATH%\%%k"
)
for %%k in (Examples,Docs,Contrib) do (
	mkdir "%PACK_PATH%\%%k"
	mkdir "%PACK_PATH%\%%k\StdUtils"
)
for %%c in (%CONFIG_NAMES%) do (
	mkdir "%PACK_PATH%\Plugins\%%c"
	copy /Y "%~dp0\Plugins\Release_%%c\*.dll" "%PACK_PATH%\Plugins\%%c"
)
copy /Y "%~dp0\Include\*.nsh"               "%PACK_PATH%\Include"
copy /Y "%~dp0\Examples\StdUtils\*.nsi"     "%PACK_PATH%\Examples\StdUtils"
copy /Y "%~dp0\Docs\StdUtils\StdUtils.html" "%PACK_PATH%\Docs\StdUtils"
copy /Y "%~dp0\*.txt"                       "%PACK_PATH%"
REM -------------------------------------------------------------------------
pushd "%~dp0"
git.exe archive --verbose --output "%PACK_PATH%\Contrib\StdUtils\StdUtils.%ISO_DATE%.Sources.tar" HEAD
popd
REM -------------------------------------------------------------------------
echo StdUtils plug-in for NSIS>                               "%PACK_PATH%\BUILD_TAG.txt"
echo Copyright (C) 2004-2018 LoRd_MuldeR ^<MuldeR2@GMX.de^>>> "%PACK_PATH%\BUILD_TAG.txt"
echo.>>                                                       "%PACK_PATH%\BUILD_TAG.txt"
echo Built on %DATE%, at %TIME%.>>                            "%PACK_PATH%\BUILD_TAG.txt"
REM -------------------------------------------------------------------------
pushd "%PACK_PATH%"
"%~dp0\Contrib\StdUtils\utils\Zip.exe" -r -9 -z "%~dp0\StdUtils.%ISO_DATE%.zip" "*.*" < "%PACK_PATH%\BUILD_TAG.txt"
popd
attrib +r "%~dp0\StdUtils.%ISO_DATE%.zip" 
rmdir /Q /S "%PACK_PATH%"
REM -------------------------------------------------------------------------
pause
