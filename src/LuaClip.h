#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#include <windows.h>
#include <winable.h>
#include <wininet.h>
#include <commctrl.h>
#include <process.h>
#include <psapi.h>
#include <tchar.h>
#include <shlwapi.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "lua\lua.hpp"

template<typename Type>
class Global
{
public:
	Global() : value(0) {}
	Global(Type v) : value(v) {}
	void operator = (Type v) { value = v; }
	operator Type() const { return value; }
private:
	Type value;
};

TCHAR szClassName[]		    = _T("LuaClipboard");
TCHAR szName[]			    = _T("LuaClip v1.4.1");

#define LUACLIP_VERSION 1410

Global <HANDLE> hMutex		= NULL;			//单实例锁
Global <bool> auto_track	= true;			//是否自动跟踪窗口
Global <HHOOK> mouse_hook	= NULL;			//低级鼠标钩子
Global <bool> is_enabled	= true;			//是否启动功能
Global <bool> disable_once	= false;		//临时禁用监控

#include "Character.cpp"        // 编码转换
#include "Utility.cpp"          // 实用函数
#include "TrayIcon.cpp"			// 托盘按钮
#include "LuaScript.cpp"		// Lua引擎

LuaScript script;
TrayIcon tray;

//因为lua不支持Unicode，我使用了UTF8来保证信息的完整性
//这里改造fopen函数，使得传入的UTF8编码路径正确识别
FILE *fopen(const char *path, const char *mode)
{
	return _wfopen(UnicodeFromUTF8(path), UnicodeFromUTF8(mode));
}
