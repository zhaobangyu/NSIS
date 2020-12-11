Push ${APP_FILE_COUNT}
Push 1
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\api-dll\*.*" 

Push ${APP_FILE_COUNT}
Push 2
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\config\*.*" 

Push ${APP_FILE_COUNT}
Push 3
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\dll-1\*.*" 

Push ${APP_FILE_COUNT}
Push 4
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\dll-2\*.*" 

Push ${APP_FILE_COUNT}
Push 5
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\dll-3\*.*" 

Push ${APP_FILE_COUNT}
Push 6
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\exe\*.*" 

Push ${APP_FILE_COUNT}
Push 7
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\html" 

Push ${APP_FILE_COUNT}
Push 8
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\QQMusicAddin" 

Push ${APP_FILE_COUNT}
Push 9
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\resae" 

Push ${APP_FILE_COUNT}
Push 10
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\sys\*.*"

Push ${APP_FILE_COUNT}
Push 11
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\TXSSO" 

