#include "Parsekeys.cpp"

int error_handler(lua_State *L)
{
	DbgPrint(L"[LuaClip Error] %s", (wchar_t*)UnicodeFromUTF8(luaL_checkstring(L, -1)));
	lua_pop(L, 1);
	return 0;
}

int message_box(lua_State *L)
{
	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	const char* text = luaL_checkstring(L, 1);

	#ifdef UNICODE
	const char* title = lua_isnoneornil(L, 2)?UTF8FromUnicode(szName):luaL_checkstring(L, 2);
	#else
	const char* title = lua_isnoneornil(L, 2)?szName:luaL_checkstring(L, 2);
	#endif
	int flag = lua_isnoneornil(L, 3)?0:luaL_checkint(L, 3);
	SetForegroundWindow(hwnd);
	int ret = MessageBoxW(0, UnicodeFromUTF8(text), UnicodeFromUTF8(title), flag);
	lua_pushnumber(L, ret);
	return 1;
}

int dbg_print(lua_State *L)
{
	luaL_Buffer buffer;
	luaL_buffinit(L, &buffer);

	int n = lua_gettop(L);
	for(int i=0;i<n;i++)
	{
		const char* text = luaL_checkstring(L, i+1);
		if(i!=0)
		{
			luaL_addstring(&buffer, "\t");
		}
		luaL_addstring(&buffer, text);
	}

	luaL_pushresult(&buffer);
	DbgPrint(L"[LuaClip] %s", (wchar_t*)UnicodeFromUTF8(luaL_checkstring(L, -1)));
	return 0;
}

int exit_process(lua_State *L)
{
	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	DestroyWindow(hwnd);
	return 0;
}

int restart(lua_State *L)
{
	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	//释放单实例锁
	ReleaseMutex(hMutex);
	CloseHandle(hMutex);

	//启动进程
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	create_process(path);

	DestroyWindow(hwnd);
	return 0;
}

int set_clipboard(lua_State *L)
{
	const char* text = luaL_checkstring(L, 1);
	bool disable = lua_isboolean(L, 2)?lua_toboolean(L, 2):true;

	SetClipboard(text, disable);

	return 0;
}

int send_text(lua_State *L)
{
	const char* text = luaL_checkstring(L, 1);
	bool track = lua_isboolean(L, 2)?lua_toboolean(L, 2):true;

	if(track)
	{
		//使用自动更新窗口
		lua_getglobal(L, "LAST_HWND");
		HWND last_hwnd = (HWND)lua_touserdata(L, -1);
		lua_pop(L, 1);
		SetForegroundWindow(last_hwnd);
	}
	else
	{
		//使用当前窗口
	}

	Sleep(50);

	HWND target_hwnd = GetForegroundWindow();

	wchar_t class_name[MAX_PATH];
	GetClassNameW(target_hwnd, class_name, MAX_PATH);
	if( wcscmp(class_name, L"ConsoleWindowClass")==0 )
	{
		//控制台窗口，不使用复制粘贴的手段
		wchar_t *str = wcsdup(UnicodeFromUTF8(text));
		for(unsigned int i=0; i<wcslen(str); i++)
		{
			::SendMessage(target_hwnd, WM_IME_CHAR, str[i], 1);
		}
		free(str);
	}
	else
	{
		//备份当前剪贴板数据
		char* str = NULL;
		if(OpenClipboard(NULL))
		{
			if(IsClipboardFormatAvailable(CF_UNICODETEXT))
			{
				HGLOBAL hglb = GetClipboardData(CF_UNICODETEXT);
				if(hglb)
				{
					LPWSTR lptstr = (LPWSTR)GlobalLock(hglb);
					if(lptstr)
					{
						str = strdup(UTF8FromUnicode(lptstr));
					}
					::GlobalUnlock(hglb);
				}

			}
			CloseClipboard();
		}

		SetClipboard(text, true);

		//发送Ctrl+V
		Sleep(20);
		keybd_event(VK_CONTROL,0,0,0);
		keybd_event('V',0,0,0);//Scan code
		keybd_event('V',0,KEYEVENTF_KEYUP,0);
		keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);

		//还原剪贴板
		Sleep(20);
		if(str)
		{
			SetClipboard(str, true);
		}
		else
		{
			OpenClipboard(NULL);
			EmptyClipboard();
			CloseClipboard();
		}

	}

	return 0;
}

int register_hotkey(lua_State *L)
{
	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	const char* keys = luaL_checkstring(L, 1);

	if(lua_isfunction(L, -1))
	{
		UINT flag = ParseHotkeys(keys);

		int ref = luaL_ref(L, LUA_REGISTRYINDEX);

		if(RegisterHotKey(hwnd, ref, LOWORD(flag), HIWORD(flag)))
		{
			lua_pushnumber(L, ref);
			return 1;
		}
	}

	return 0;
}

int unregister_hotkey(lua_State *L)
{
	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	int ref = luaL_checknumber(L, 1);
	UnregisterHotKey(hwnd, ref);

	return 0;
}

int check_auto_run(lua_State *L)
{
	lua_pushboolean(L, CheckAutoRun());
	return 1;
}

int set_auto_run(lua_State *L)
{
	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	SetAutoRun(hwnd);

	return 0;
}

int track_popup_menu(lua_State *L)
{
	auto_track = false;//弹出快捷菜单时禁用自动跟踪窗口

	lua_getglobal(L, "HWND");
	HWND hwnd = (HWND)lua_touserdata(L, -1);
	lua_pop(L, 1);

	POINT pt;
	get_caret_pos(&pt);

	HWND last_hwnd = GetForegroundWindow();
	lua_pushlightuserdata(L, last_hwnd);
	lua_setglobal(L, "LAST_HWND");

	HMENU hMenu = CreatePopupMenu();

	ShowContextMenu(L, hMenu);

	//Track menu
	SetForegroundWindow(hwnd);
	TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
	DestroyMenu(hMenu);

	auto_track = true;
	return 0;
}

int get_window_info(lua_State *L)
{
	bool foreground = lua_isboolean(L, 1)?lua_toboolean(L, 1):false;

	HWND temp = 0;
	if(foreground)
	{
		temp = GetForegroundWindow();
	}
	else
	{
		POINT pt;
		GetCursorPos(&pt);
		temp = WindowFromPoint(pt);
	}

	wchar_t title_name[MAX_PATH];
	wchar_t class_name[MAX_PATH];
	wchar_t file_path[MAX_PATH];

	GetWindowTextW(temp, title_name, MAX_PATH);
	GetClassNameW(temp, class_name, MAX_PATH);

	DWORD processID = 0;
	GetWindowThreadProcessId(temp, &processID);

	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID );
	GetProcessImageFileNameW(hProcess, file_path, MAX_PATH);
	CloseHandle(hProcess);

	DevicePathToWin32Path(file_path);

	lua_pushstring(L, UTF8FromUnicode(title_name));
	lua_pushstring(L, UTF8FromUnicode(class_name));
	lua_pushstring(L, UTF8FromUnicode(file_path));
	return 3;
}

int list_plugins(lua_State *L)
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	PathRemoveFileSpecW(path);
	wcscat(path, L"\\plugins\\*.*");

	int index = 1;
	lua_newtable(L);

	WIN32_FIND_DATAW ffbuf;
	HANDLE hfind = FindFirstFileW(path, &ffbuf);
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if( isEndWith(ffbuf.cFileName, L".lua") || isEndWith(ffbuf.cFileName, L".luac") )
			{
				wchar_t *p = wcsrchr(ffbuf.cFileName, '.');
				if(p) *p = 0;

				lua_pushnumber(L, index);
				lua_pushstring(L, UTF8FromUnicode(ffbuf.cFileName));
				lua_settable(L, -3);
				index++;
			}
		}
		while (FindNextFileW(hfind, &ffbuf));
		FindClose(hfind);
	}
	return 1;
}

int execute(lua_State *L)
{
	const char* path = luaL_checkstring(L, 1);
	int delay = lua_isnoneornil(L, 2)?0:luaL_checkint(L, 2);

	if(wcslen(UnicodeFromUTF8(path))>=MAX_PATH) luaL_error(L, "path is too long");

	thread_content *cxt = (thread_content*)malloc(sizeof(thread_content));
	cxt->delay = delay * 1000;//毫秒转换为秒
	wcscpy(cxt->path, UnicodeFromUTF8(path));

	_beginthread(create_process_delay, 0, (LPVOID)cxt);

	return 0;
}

int shell_execute(lua_State *L)
{
	const char* path = luaL_checkstring(L, 1);

	ShellExecute(NULL, L"open", UnicodeFromUTF8(path), NULL, NULL, SW_SHOWNORMAL);

	return 0;
}

#if 0
static void stackDump (lua_State *L) {
      int i;
      int top = lua_gettop(L);
      for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {

          case LUA_TSTRING:  /* strings */
            DbgPrint(L"`%S'", lua_tostring(L, i));
            break;

          case LUA_TBOOLEAN:  /* booleans */
            DbgPrint(L"%S",lua_toboolean(L, i) ? "true" : "false");
            break;

          case LUA_TNUMBER:  /* numbers */
            DbgPrint(L"%g", lua_tonumber(L, i));
            break;

          default:  /* other values */
            DbgPrint(L"%S", lua_typename(L, t));
            break;

        }
      }
    }
#endif // 0

void ThreadProc(LPVOID lpParameter)
{
	lua_State* L = (lua_State*)lpParameter;

	luaL_checktype(L, 1, LUA_TFUNCTION);

	if(lua_pcall(L, lua_gettop(L)-1, 0, 0))
	{
		error_handler(L);
	}

	lua_pushlightuserdata(L, L);
	lua_pushnil(L);
	lua_settable(L, LUA_REGISTRYINDEX);
}

int create_thread(lua_State *L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);

	lua_State *Thread = lua_newthread(L);

	lua_pushlightuserdata(L, Thread);
	lua_insert(L, -2);
	lua_settable(L, LUA_REGISTRYINDEX);

	lua_xmove(L, Thread, lua_gettop(L));

	_beginthread(ThreadProc, 0, (LPVOID)Thread);

	return 0;
}

int get_http(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	const char *url = luaL_checkstring(L, 2);
	const char *post = lua_isnoneornil(L, 3)?"":luaL_checkstring(L, 3);

	BYTE *m_data = 0;

	//解析url
	char host[MAX_PATH+1];
	char path[MAX_PATH+1];

	URL_COMPONENTSA uc = {0};
	uc.dwStructSize = sizeof(uc);

	uc.lpszHostName = host;
	uc.dwHostNameLength = MAX_PATH;

	uc.lpszUrlPath = path;
	uc.dwUrlPathLength = MAX_PATH;

	::InternetCrackUrlA(url, 0, ICU_ESCAPE, &uc);

	HINTERNET hInet = ::InternetOpenA("Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; Trident/5.0)", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInet)
	{
		HINTERNET hConn = ::InternetConnectA(hInet, host, uc.nPort, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 1);
		if (hConn)
		{
			HINTERNET hRes = ::HttpOpenRequestA(hConn, lua_isnoneornil(L, 3)?"GET":"POST", path, 0, NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, 1);
			if (hRes)
			{
				const char szHeader[] = "Content-Type: application/x-www-form-urlencoded\r\n";
				if ( ::HttpSendRequestA(hRes, szHeader, strlen(szHeader), (LPVOID *)post, strlen(post)) )
				{
					DWORD dwTotal = 0;
					while(1)
					{
						DWORD dwLength = 0;
						DWORD dwByteRead = 0;

						if ( ::InternetQueryDataAvailable(hRes, &dwLength, 0, 0) && dwLength)
						{
							m_data = (BYTE*)realloc(m_data, dwTotal + dwLength + 1);

							if( ::InternetReadFile(hRes, m_data + dwTotal, dwLength, &dwByteRead) )
							{
								dwTotal += dwLength;
							}
							else
							{
								break;
							}
						}
						else
						{
							break;
						}
					}

					if(dwTotal)
					{
						m_data[dwTotal] = '\0';// 补充一个字符串结束符，方便打印查看

						lua_settop(L, 1);
						lua_pushlstring(L, (char*)m_data, dwTotal);
						if(lua_pcall(L, 1, 0, 0))
						{
							error_handler(L);
						}

						free(m_data);
					}
				}

				::InternetCloseHandle(hRes);
			}

			::InternetCloseHandle(hConn);
		}

		::InternetCloseHandle(hInet);
	}
	return 0;
}

int sleep(lua_State *L)
{
	int delay = luaL_checkint(L, 1);

	Sleep(delay);

	return 0;
}
