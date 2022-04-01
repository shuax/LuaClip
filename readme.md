# 欢迎使用LuaClip

## 警告
此项目已经废弃！！！

## 介绍
LuaClip是一款window剪贴板增强软件，目前并不成熟还需要您的各种建议

程序启动以后会在任务栏通知区域显示一个图标，点击右键可弹出菜单，点击左键可临时禁用软件

按下ctrl+alt+v可在光标位置弹出快捷粘贴菜单，使用ctrl+1 - ctrl+0 可快速粘贴1-10号剪贴板内容

## 特性
- 我们使用Lua语言来作为扩展系统，可轻松修改脚本
- 主程序采用Unicode编码，Lua脚本采用UTF8编码，支持各种系统不乱码

## 基础
- Lua基础脚本需要提供这些函数处理来自主程序的消息
	- `OnRightButtonDown()` 右键点击托盘图标时调用
	- `OnClipboardChange(text)` 剪贴板变化时调用，目前仅支持文本类型
- 主程序为Lua虚拟机提供了这些实用函数
	- `int MessageBox(text, title="", flag=0)` 显示对话框，返回确认类型
	- `void print(...)` 打印调试信息，可使用[DebugView](http://technet.microsoft.com/en-us/sysinternals/bb896647)工具查看
	- `void ExitProcess()` 退出程序
	- `void Restart()` 重启程序
	- `void SetClipboard(text, disable=true)` 设置剪贴板内容，disable禁止监控本次剪贴板操作
	- `void SendText(text, track=true)` 发送文本窗口，track=true发送到自动跟踪窗口，否则发送到当前窗口
	- `int RegisterHotKey(keys, func)` 注册热键到关联函数，成功返回注册句柄
	- `void UnregisterHotKey(handle)` 释放之前关联的热键
	- `bool CheckAutoRun()` 本程序是否开机自启动
	- `void SetAutoRun()` 切换开机自启动状态
	- `void PopupMenu(menu)` 在光标所在位置弹出菜单，无法找到光标则在鼠标指针所在位置弹出
	- `title, class, path = GetWindowInfo(foreground=false)` 查询窗口信息，依次返回窗口标题、窗口类名、窗口进程名，foreground表示查询当前窗口还是鼠标位置窗口
	- `list ListPlugins()` 列出插件目录下所有`.lua`或`.luac`文件，返回值是table类型
	- `void Execute(path, delay=0)` 延迟delay秒启动一个可执行程序
	- `void ShellExecute(path)` 打开网址、文件等
	- `void CreateThread(func, ...)` 创建一个线程，`...`为func的参数
	- `void Http(func, url, post="")` 发送http请求，完成后会自动调用func(str)
	- `void Sleep(millisecond)` 挂起一定时间，单位毫秒
- 这部分脚本放在scripts目录中，主程序启动会自动载入LuaClip.lua

## 插件（开发中）
- 插件系统也使用Lua语言编写，需要放在plugins目录下，这个系统由基础脚本驱动。
	- `plugin.Init()` 启用插件时会自动调用
	- `plugin.Uninit()` 停止插件时会自动调用
	- `RegisterPlugin(plugin)` 注册一个插件
- 基础脚本为插件系统提供了以下回调函数，通常回调函数返回true表示插件已经处理此消息，基础脚本不再继续处理
	- `BeforeClipboardChange(text)` 剪贴板发生改变，且尚未加入历史记录
	- `AfterClipboardChange(text)` 剪贴板发生改变，且已经加入历史记录
