# 这是NSIS打包的完整项目代码 #

# 前言 #

> 项目示例包括QQ音乐、网易云音乐、ScreenToGif三个安装包的示例

> QQ音乐与网易云音乐这2个安装包比较简单，主要是界面上的差异

> ScreenToGif这个示例安装包，在安装前做了net.framework版本的验证，net.framework版本低于4.8则下载环境进行安装

> 项目中的关键参数使用了宏定义，你可以查看 位于 AppCode\项目\app\setup.nsi 的文件


    提示： net.framework 4以后的版本安装需要先安装微软证书，再安装依赖包才能安装成功

### 由于github文件限制大小的原因，安装包没有上传，可执行目录中的脚本进行生成。
#### 三个示例包对应的脚本分别为 ####
	
	QQ音乐：build-qqmusic-nozip.bat
	网易云音乐：build-cloudmusic-nozip.bat
	ScreenToGif：build-screentogif-nozip.bat

### 执行脚本前的改动 ###
例如QQ音乐的安装，你需要打开 AppCode\QQMusic\app\setup.nsi，找到 APP_FILE_DIR 这个宏定义，将值修改为 AppFile 实际存在的位置。

当然如果你不想修改任何内容，可以将该项目下载到D:\\github\\NSIS\\NsisPackage目录下。

## 你可以在下面看到三个安装包安装过程的gif动画录制 ##

QQ音乐安装过程
![image](https://github.com/zhaobangyu/NSIS/blob/NsisPackage/qqmusic-install.gif)  

网易云安装过程
![image](https://github.com/zhaobangyu/NSIS/blob/NsisPackage/cloudmusic-install.gif)  

ScreenToGif安装过程
![image](https://github.com/zhaobangyu/NSIS/blob/NsisPackage/screento-install.png)  
