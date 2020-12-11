Push ${APP_FILE_COUNT}
Push 1  
Call ExtractCallback 
File "${APP_FILE_DIR}\ScreenToGif.70x70.png" 

Push ${APP_FILE_COUNT}
Push 2  
Call ExtractCallback 
File "${APP_FILE_DIR}\ScreenToGif.150x150.png" 

Push ${APP_FILE_COUNT}
Push 3 
Call ExtractCallback 
File "${APP_FILE_DIR}\Settings.xaml" 

Push ${APP_FILE_COUNT}
Push 4  
Call ExtractCallback 
File "${APP_FILE_DIR}\ScreenToGif.visualelementsmanifest.xml" 

Push ${APP_FILE_COUNT}
Push 5  
Call ExtractCallback 
File "${APP_FILE_DIR}\ScreenToGif.exe" 

Push ${APP_FILE_COUNT}
Push 6  
Call ExtractCallback 
File "${APP_FILE_DIR}\gifski.dll" 

Push ${APP_FILE_COUNT}
Push 7  
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpDX.Direct3D11.dll" 

Push ${APP_FILE_COUNT}
Push 8  
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpDX.dll" 

Push ${APP_FILE_COUNT}
Push 9  
Call ExtractCallback 
File "${APP_FILE_DIR}\SharpDX.DXGI.dll" 