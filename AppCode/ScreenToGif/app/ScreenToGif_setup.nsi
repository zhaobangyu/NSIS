# ====================== 自定义宏 产品信息==============================
!define PRODUCT_NAME           		"ScreenToGif"
#安装卸载项用到的KEY
!define PRODUCT_PATHNAME 			"ScreenToGif"
#安装路径追加的名称 
!define INSTALL_APPEND_PATH         "ScreenToGif"
#默认生成的安装路径 
!define INSTALL_DEFALT_SETUPPATH    ""
#执行文件名称 
!define EXE_NAME               		"ScreenToGif.exe"
#版本号
!define PRODUCT_VERSION        		"1.0.0.0"
#主页地址
!define HOME_URL    		        "https://www.screentogif.com/"
#用户条款
!define TERMS_URL    		        ""
#产品发布商
!define PRODUCT_PUBLISHER      		"Nicke Manarin"
#产品法律
!define PRODUCT_LEGAL          		"Nicke Manarin Copyright（c）2020"
#打包出来的文件名称
!define INSTALL_OUTPUT_NAME    		"ScreenToGif_${PRODUCT_VERSION}.exe"
#应用程序的数据目录
!define LOCAL_APPDATA_DIR    		"$LOCALAPPDATA\ScreenToGif"
#打包文件目录
!define APP_FILE_DIR    		    "D:\github\NSISPackage\AppFile\ScreenToGif"
#文件数量
!define APP_FILE_COUNT    		    9
#完整安装包下载地址
!define ALL_SETUP_DL_URL            ""
#Net包名称
!define NET_PACK_NAME               "ndp48-x86-x64-allos-enu.exe"
#Net包下载地址
!define NET_PACK_DL_URL             "https://download.visualstudio.microsoft.com/download/pr/014120d7-d689-4305-befd-3cb711108212/0fd66638cde16859462a6243a4629a50/ndp48-x86-x64-allos-enu.exe"
#微软证书名称(win7安装net4.6以上版本需下载微软证书并安装,否则net安装会失败)
#net4.0不需要安装微软证书
!define MS_ROOT_CERT_NAME           "MicrosoftRootCertificateAuthority2011.cer"
#微软证书下载地址
!define MS_ROOT_CERT_DL_URL         "https://download.microsoft.com/download/2/4/8/248D8A62-FCCD-475C-85E7-6ED59520FC0F/MicrosoftRootCertificateAuthority2011.cer"


# ====================== 自定义宏 安装信息==============================
!define INSTALL_7Z_PATH 	   		"..\app.7z"
!define INSTALL_7Z_NAME 	   		"app.7z"
!define INSTALL_RES_PATH       		"skin.zip"
!define INSTALL_LICENCE_FILENAME    "licence.rtf"
!define INSTALL_ICO 				"logo.ico"
!define UNINSTALL_ICO 				"uninst.ico"

#SetCompressor lzma

!include "ScreenToGif_setup.nsh"

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
