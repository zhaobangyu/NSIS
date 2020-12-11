# Inetc #
## Inetc客户端插件，用于文件的下载和上传 ##

## 默认使用方法 ##
    inetc::get "https://downloadUrl" "$TEMP\ndp48-x86-x64-allos-enu.exe"

## 扩展后的使用方法 ##

	; Net安装包下载回调
	Function NetPackDownLoadCallBack
		; 0-当前进度（百分比）
		Pop $0
		; 1-累计大小
		Pop $1
		; 2-已下载大小
		Pop $2
		; 3-下载速度
		Pop $3
		; 4-剩余时间
		Pop $4
	FunctionEnd

	;下载 .NET Framework 4.0
	Function DownloadNetFramework4
		GetFunctionAddress $R9 NetPackDownLoadCallBack
		inetc::get "${NET_PACK_DL_URL}" "$TEMP\${NET_PACK_NAME}" $R9
		; 读取值
		Pop $1
		; 写入值
		Push $1
	FunctionEnd
