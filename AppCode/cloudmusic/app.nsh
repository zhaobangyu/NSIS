Push ${APP_FILE_COUNT}
Push 1
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\locales"

Push ${APP_FILE_COUNT}
Push 2
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\package"

Push ${APP_FILE_COUNT}
Push 3
Call ExtractCallback 
; 注：匹配使用目录匹配，安装的目录与原目录一致
File /r "${APP_FILE_DIR}\resource"

Push ${APP_FILE_COUNT}
Push 4
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\exe\*.*" 

Push ${APP_FILE_COUNT}
Push 5
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\dll\*.*" 

Push ${APP_FILE_COUNT}
Push 6
Call ExtractCallback 
; 注：匹配使用*.*则将所匹配到的文件安装到$INSTDIR目录下
File /r "${APP_FILE_DIR}\config\*.*" 