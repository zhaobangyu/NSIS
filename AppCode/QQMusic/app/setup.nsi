# ====================== 自定义宏 产品信息==============================
!define PRODUCT_NAME           		"QQ音乐"
#安装卸载项用到的KEY
!define PRODUCT_PATHNAME 			"QQMusic"
#安装路径追加的名称 
!define INSTALL_APPEND_PATH         "QQMusic"
#默认生成的安装路径 
!define INSTALL_DEFALT_SETUPPATH    ""
#执行文件名称 
!define EXE_NAME               		"QQMusic.exe"
#版本号
!define PRODUCT_VERSION        		"1.0.0.0"
#主页地址
!define HOME_URL    		        "https://y.qq.com/"
#用户条款
!define TERMS_URL    		        "https://y.qq.com/y/static/tips/service_tips.html"
#产品发布商
!define PRODUCT_PUBLISHER      		"腾讯公司"
#产品法律
!define PRODUCT_LEGAL          		"腾讯公司 Copyright（c）2020"
#打包出来的文件名称
!define INSTALL_OUTPUT_NAME    		"QQMusic${PRODUCT_VERSION}.exe"
#应用程序的数据目录
!define LOCAL_APPDATA_DIR    		"$LOCALAPPDATA\QQMusic"
#打包文件目录
!define APP_FILE_DIR    		    "D:\github\NSISPackage\AppFile\QQMusic"
#文件数量
!define APP_FILE_COUNT    		    12

# ====================== 自定义宏 安装信息==============================
!define INSTALL_7Z_PATH 	   		"..\app.7z"
!define INSTALL_7Z_NAME 	   		"app.7z"
!define INSTALL_RES_PATH       		"skin.zip"
!define INSTALL_LICENCE_FILENAME    "licence.rtf"
!define INSTALL_ICO 				"logo.ico"
!define UNINSTALL_ICO 				"uninst.ico"

#SetCompressor lzma

!include "setup.nsh"

# ==================== NSIS属性 ================================

# 针对Vista和win7 的UAC进行权限请求.
# RequestExecutionLevel none|user|highest|admin
RequestExecutionLevel admin

; 安装包名字.
Name "${PRODUCT_NAME}"

# 安装程序文件名.

OutFile "..\..\..\Output\${INSTALL_OUTPUT_NAME}"

;$PROGRAMFILES32\Netease\NIM\

InstallDir "1"

# 安装和卸载程序图标
Icon              "${INSTALL_ICO}"
UninstallIcon     "${UNINSTALL_ICO}"
