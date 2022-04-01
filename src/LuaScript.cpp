#include "LuaBind.cpp"

class LuaScript
{
public:
	LuaScript()
	:L(NULL)
	{
		L = luaL_newstate();
		luaL_openlibs(L);
		lua_atpanic(L, error_handler);

		lua_pushcfunction(L, message_box);
		lua_setglobal(L, "MessageBox");

		lua_pushcfunction(L, dbg_print);
		lua_setglobal(L, "print");

		lua_pushcfunction(L, exit_process);
		lua_setglobal(L, "ExitProcess");

		lua_pushcfunction(L, restart);
		lua_setglobal(L, "Restart");

		lua_pushcfunction(L, set_clipboard);
		lua_setglobal(L, "SetClipboard");

		lua_pushcfunction(L, send_text);
		lua_setglobal(L, "SendText");

		lua_pushcfunction(L, register_hotkey);
		lua_setglobal(L, "RegisterHotKey");

		lua_pushcfunction(L, unregister_hotkey);
		lua_setglobal(L, "UnregisterHotKey");

		lua_pushcfunction(L, check_auto_run);
		lua_setglobal(L, "CheckAutoRun");

		lua_pushcfunction(L, set_auto_run);
		lua_setglobal(L, "SetAutoRun");

		lua_pushcfunction(L, track_popup_menu);
		lua_setglobal(L, "PopupMenu");

		lua_pushcfunction(L, get_window_info);
		lua_setglobal(L, "GetWindowInfo");

		lua_pushcfunction(L, list_plugins);
		lua_setglobal(L, "ListPlugins");

		lua_pushcfunction(L, execute);
		lua_setglobal(L, "Execute");

		lua_pushcfunction(L, shell_execute);
		lua_setglobal(L, "ShellExecute");

		lua_pushcfunction(L, create_thread);
		lua_setglobal(L, "CreateThread");

		lua_pushcfunction(L, get_http);
		lua_setglobal(L, "Http");

		lua_pushcfunction(L, sleep);
		lua_setglobal(L, "Sleep");

		luaL_dostring(L, "package.path=[[.\\?.lua]]");
		luaL_dostring(L, "package.cpath=[[]]");

		lua_pushnumber(L, LUACLIP_VERSION);
		lua_setglobal(L, "LUACLIP_VERSION");
	}

	bool Load(const char *path)
	{
		if(luaL_dofile(L, path))
		{
			error_handler(L);
			return false;
		}
		return true;
	}

	void Init(HWND hwnd)
	{
		lua_pushlightuserdata(L, hwnd);
		lua_setglobal(L, "HWND");
	}

	void OnRightButtonDown(HWND hwnd)
	{
		auto_track = false;//弹出菜单时禁用自动跟踪窗口

		POINT pt;
		GetCursorPos(&pt);
		HMENU hMenu = CreatePopupMenu();

		lua_getglobal(L, "OnRightButtonDown");
		if(lua_isfunction(L, -1))
		{
			if(lua_pcall(L, 0, 1, 0))
			{
				error_handler(L);
			}
		}

		ShowContextMenu(L, hMenu);

		if(GetMenuItemCount(hMenu)==0)
		{
			AppendMenuW(hMenu, MF_BYPOSITION | MF_STRING, MENU_EXIT, L"退出程序(&E)");
		}

		//
		SetForegroundWindow(hwnd);
		TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
		DestroyMenu(hMenu);

		auto_track = true;
	}

	void TrackActiveWnd()
	{
		if(auto_track)
		{
			static HWND last_hwnd = 0;

			HWND temp = GetForegroundWindow();
			if(last_hwnd!=temp)
			{
				last_hwnd = temp;
				lua_pushlightuserdata(L, last_hwnd);
				lua_setglobal(L, "LAST_HWND");
			}
		}
	}

	void SendText(const char* str)
	{
		lua_getglobal(L, "OnClipboardChange");
		if(lua_isfunction(L, -1))
		{
			lua_pushstring(L, str);
			if(lua_pcall(L, 1, 0, 0))
			{
				error_handler(L);
			}
		}
	}

	void DispatchMessage(int msg)
	{
		lua_rawgeti(L , LUA_REGISTRYINDEX , msg);
		if(lua_isfunction(L, -1))
		{
			if(lua_pcall(L, 0, 0, 0))
			{
				error_handler(L);
			}
		}
	}
private:
	lua_State *L;
};
