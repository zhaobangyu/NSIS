Push ${APP_FILE_COUNT}
Push 1  
Call ExtractCallback 
File "${APP_FILE_DIR}\KByte.exe" 

Push ${APP_FILE_COUNT}
Push 2  
Call ExtractCallback 
File "${APP_FILE_DIR}\updater.exe" 

Push ${APP_FILE_COUNT}
Push 3
Call ExtractCallback 
File "${APP_FILE_DIR}\uninst.exe"

Push ${APP_FILE_COUNT}
Push 4  
Call ExtractCallback 
File "${APP_FILE_DIR}\HandyControl.dll" 

Push ${APP_FILE_COUNT}
Push 5  
Call ExtractCallback 
File "${APP_FILE_DIR}\kklibrary.dll" 

Push ${APP_FILE_COUNT}
Push 6  
Call ExtractCallback 
File "${APP_FILE_DIR}\LiveCharts.dll" 

Push ${APP_FILE_COUNT}
Push 7  
Call ExtractCallback 
File "${APP_FILE_DIR}\LiveCharts.Wpf.dll" 

Push ${APP_FILE_COUNT}
Push 8  
Call ExtractCallback 
File "${APP_FILE_DIR}\Microsoft.Expression.Interactions.dll" 

Push ${APP_FILE_COUNT}
Push 9  
Call ExtractCallback 
File "${APP_FILE_DIR}\Microsoft.Windows.Shell.dll"

Push ${APP_FILE_COUNT}
Push 10  
Call ExtractCallback 
File "${APP_FILE_DIR}\Newtonsoft.Json.dll"  

Push ${APP_FILE_COUNT}
Push 11 
Call ExtractCallback 
File "${APP_FILE_DIR}\ProxyLibrary.dll"

Push ${APP_FILE_COUNT}
Push 12
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Converters.Wpf.dll"

Push ${APP_FILE_COUNT}
Push 13  
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Core.dll"

Push ${APP_FILE_COUNT}
Push 14 
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Css.dll"

Push ${APP_FILE_COUNT}
Push 15 
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Dom.dll"

Push ${APP_FILE_COUNT}
Push 16 
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Model.dll"

Push ${APP_FILE_COUNT}
Push 17  
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Rendering.Gdi.dll"

Push ${APP_FILE_COUNT}
Push 18 
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Rendering.Wpf.dll"

Push ${APP_FILE_COUNT}
Push 19  
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpVectors.Runtime.Wpf.dll"

Push ${APP_FILE_COUNT}
Push 20 
Call ExtractCallback 
File "${APP_FILE_DIR}\System.Windows.Interactivity.dll"

SetOutPath "$INSTDIR\socks" 
Push ${APP_FILE_COUNT}
Push 21
Call ExtractCallback 
File "${APP_FILE_DIR}\socks\kksocksclr4.dll"

SetOutPath "$INSTDIR" 
Push ${APP_FILE_COUNT}
Push 22
Call ExtractCallback 
File "${APP_FILE_DIR}\KByte.exe.config" 

Push ${APP_FILE_COUNT}
Push 23
Call ExtractCallback 
File "${APP_FILE_DIR}\updater.exe.config" 

Push ${APP_FILE_COUNT}
Push 24
Call ExtractCallback 
File "${PLUG_IN_FILE_DIR}\MicrosoftRootCertificateAuthority2011.cer"

Push ${APP_FILE_COUNT}
Push 25
Call ExtractCallback 
File "${PLUG_IN_FILE_DIR}\NDP472-KB4054530-x86-x64-AllOS-ENU.exe"