!ifdef NSIS_UNICODE
!AddPluginDir .\ReleaseU
!else
!AddPluginDir .\ReleaseA
!endif

Name "ThreadTimer"
OutFile "ThreadTimer.exe"

Page instfiles

XPStyle on
ShowInstDetails show

Section Install
    GetFunctionAddress $0 FuncTimer
    ThreadTimer::Start 1000 10 $0
SectionEnd

Function FuncTimer
    IntOp $R0 $R0 + 1
    DetailPrint "FuncTimer has been called for $R0 time(s)!"
FunctionEnd