# Inetc #
## Inetc客户端插件，用于文件的下载和上传 ##

## 默认使用方法 ##
    KillProcDLL::KillProc "process_name.exe"

## 扩展后的使用方法 ##

### SendMessageByHwnd ###
通过窗口句柄向指定进程发送命令，不推荐使用应该方法，当窗口处于隐藏状态时，使用此方法发送时，应用程序接收不到消息

### 使用示例 ###
	KillProcDLL::SendMessageByHwnd "process_name.exe" 1

### SendMessageByMainThreadId ###
通过窗口句柄向指定进程的主线程发送命令，推荐使用此方法，不受窗口的状态影响。

### 使用示例 ###
	KillProcDLL::SendMessageByMainThreadId "process_name.exe" 1

### 接收方代码示例(以C#，WPF程序为例) ###
	public MainWindow()
	{
    	//主线程消息过滤器
        ComponentDispatcher.ThreadFilterMessage += ComponentDispatcher_ThreadFilterMessage;
	}
 
	/// <summary>
	/// //主线程消息过滤器
	/// </summary>
	/// <param name="msg"></param>
	/// <param name="handled"></param>
	private void ComponentDispatcher_ThreadFilterMessage(ref MSG msg, ref bool handled)
	{
    	if (msg.message == 1)
	    {
	        //接到消息，执行对应的逻辑
	    }
	}