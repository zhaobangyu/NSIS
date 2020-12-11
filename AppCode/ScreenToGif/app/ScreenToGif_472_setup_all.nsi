# ====================== 自定义宏 产品信息==============================
!define PRODUCT_NAME           		"KByte"
#安装卸载项用到的KEY
!define PRODUCT_PATHNAME 			"KByte"
#安装路径追加的名称 
!define INSTALL_APPEND_PATH         "KByte"
#默认生成的安装路径 
!define INSTALL_DEFALT_SETUPPATH    ""
#执行文件名称 
!define EXE_NAME               		"KByte.exe"
#版本号
!define PRODUCT_VERSION        		"4.1.0.0"
#主页地址
!define HOME_URL    		        "http://www.kbytevpn.com"
#用户条款
!define TERMS_URL    		        "http://www.kbytevpn.com/terms.html"
#产品发布商
!define PRODUCT_PUBLISHER      		"浙江快快连接信息科技有限公司"
#产品法律
!define PRODUCT_LEGAL          		"浙江快快连接信息科技有限公司 Copyright（c）2020"
#打包出来的文件名称
!define INSTALL_OUTPUT_NAME    		"KByteAllSetupAll_472.exe"
#应用程序的数据目录
!define LOCAL_APPDATA_DIR    		"$LOCALAPPDATA\KByte"
#打包文件目录
!define APP_FILE_DIR    		    "D:\myCode\shadowsocks\shadowsocks-qt-client\PackageDirectory\KByte"
#打包文件目录
!define PLUG_IN_FILE_DIR    		"D:\myCode\shadowsocks\shadowsocks-qt-client\PackageDirectory\Plugin"
#Net包名称
!define NET_PACK_NAME               "NDP472-KB4054530-x86-x64-AllOS-ENU.exe"
#微软证书名称(win7安装net4.6以上版本需下载微软证书并安装,否则net安装会失败)
#net4.0不需要安装微软证书
!define MS_ROOT_CERT_NAME           "MicrosoftRootCertificateAuthority2011.cer"
#文件数量
!define APP_FILE_COUNT    		    25


# ====================== 自定义宏 安装信息==============================
!define INSTALL_7Z_PATH 	   		"..\app.7z"
!define INSTALL_7Z_NAME 	   		"app.7z"
!define INSTALL_RES_PATH       		"skin.zip"
!define INSTALL_LICENCE_FILENAME    "licence.rtf"
!define INSTALL_ICO 				"logo.ico"
!define UNINSTALL_ICO 				"uninst.ico"

#SetCompressor lzma

!include "kbyte_socks_472_setup_all.nsh"

# ==================== NSIS属性 ================================

# 针对Vista和win7 的UAC进行权限请求.
# RequestExecutionLevel none|user|highest|admin
RequestExecutionLevel admin

; 安装包名字.
Name "${PRODUCT_NAME}"

# 安装程序文件名.

OutFile "..\..\Output\${INSTALL_OUTPUT_NAME}"

;$PROGRAMFILES32\Netease\NIM\

InstallDir "1"

# 安装和卸载程序图标
Icon              "${INSTALL_ICO}"
UninstallIcon     "${UNINSTALL_ICO}"
