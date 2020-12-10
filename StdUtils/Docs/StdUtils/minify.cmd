@echo off
set "HTML_COMPRESSOR_PATH=E:\Source\Prerequisites\HTMLCompressor\bin"

if not exist "%JAVA_HOME%\bin\java.exe" (
    echo Java runtime not found, please check your JAVA_HOME!
    pause & goto:eof
)

if not exist "%HTML_COMPRESSOR_PATH%\htmlcompressor-1.5.3.jar" (
    echo HTML Compressor not found, please check your HTML_COMPRESSOR_PATH!
    pause & goto:eof
)

"%JAVA_HOME%\bin\java.exe" -jar "%HTML_COMPRESSOR_PATH%\htmlcompressor-1.5.3.jar" --compress-css -o "%~dp0\StdUtils.html" "%~dp0\StdUtils.FULL.html"

REM pause
