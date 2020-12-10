@echo off
setlocal EnableDelayedExpansion
REM ----------------------------------------------------------------------
set "NSIS_ANSI=C:\Program Files (x86)\NSIS\ANSI"
set "NSIS_Unicode=C:\Program Files (x86)\NSIS\Unicode"
REM ----------------------------------------------------------------------
set "NSIS_PROJECTS=StdUtilsTest,SHFileOperation,ShellExecAsUser,InvokeShellVerb,ShellExecWait,GetParameters,AppendToFile,HashFunctions,TimerCreate,OSVersion,PathUtils,ProtectStr"
REM ----------------------------------------------------------------------
REM
cd /D "%~dp0"
for %%i in (*.nsi) do (
	if exist "%~dp0\%%~ni-ANSI.exe" (
		del "%~dp0\%%~ni-ANSI.exe"
		if exist "%~dp0\%%~ni-ANSI.exe" (
			pause && exit
		)
	)
	if exist "%~dp0\%%~ni-Unicode.exe" (
		del "%~dp0\%%~ni-Unicode.exe"
		if exist "%~dp0\%%~ni-Unicode.exe" (
			pause && exit
		)
	)
)
REM ----------------------------------------------------------------------
for %%i in (*.nsi) do (
	echo --==[[ %%~nxi ]]==--
	echo.
	
	"%NSIS_ANSI%\makensis.exe" "%~dp0\%%~nxi"
	if not "!ERRORLEVEL!"=="0" pause && exit
	if not exist "%~dp0\%%~ni-ANSI.exe" pause && exit
	
	echo.
	echo --------
	echo.

	"%NSIS_Unicode%\makensis.exe" "%~dp0\%%~nxi"
	if not "!ERRORLEVEL!"=="0" pause && exit
	if not exist "%~dp0\%%~ni-Unicode.exe" pause && exit
	
	echo.
	echo --------
	echo.
)
REM ----------------------------------------------------------------------
pause
