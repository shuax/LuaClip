#ifndef GET_KEYSTATE_WPARAM
	#define GET_KEYSTATE_WPARAM(wParam)     (LOWORD(wParam))
#endif

#define MENU_EXIT (WM_APP + 0x100)

void DbgPrint(const wchar_t *s,...)
{
	va_list vl;
	va_start(vl, s);
	wchar_t buffer[1024];
	_vsnwprintf(buffer, 1024, s, vl);
	OutputDebugStringW(buffer);
	va_end(vl);
}

bool create_process(wchar_t *path)
{
	//启动进程
	STARTUPINFOW si = {0};
	PROCESS_INFORMATION pi = {0};

	si.cb = sizeof(STARTUPINFO);
	//GetStartupInfo(&si);

	return CreateProcessW(NULL, path, NULL, NULL, TRUE, 0, NULL, 0, &si, &pi);
}

struct thread_content
{
	wchar_t path[MAX_PATH];
	int delay;
};

void create_process_delay(LPVOID pvoid)
{
	thread_content *cxt = (thread_content*)pvoid;
	Sleep(cxt->delay);

	create_process(cxt->path);

	free(cxt);
}

void get_caret_pos(POINT *pt)
{
	GUITHREADINFO pg;
	pg.cbSize = sizeof(GUITHREADINFO);
	::GetGUIThreadInfo(0, &pg);

	if(pg.hwndCaret)
	{
		pt->x = pg.rcCaret.right;
		pt->y = pg.rcCaret.bottom;
		::ClientToScreen(pg.hwndCaret, pt);
	}
	else
	{
		GetCursorPos(pt);
	}
}

void SetClipboard(const char* text, bool disable)
{
	disable_once = disable;

	UnicodeFromUTF8 str(text);
	HGLOBAL hSetData = GlobalAlloc(GMEM_MOVEABLE, str.size());
	LPBYTE lpSetData = (LPBYTE)GlobalLock(hSetData);
	memcpy(lpSetData, (wchar_t*)str, str.size());
	GlobalUnlock( lpSetData );
	if(OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, lpSetData);
		CloseClipboard();
	}

	disable_once = false;
}

void GetPrettyPath(TCHAR *path)
{
	TCHAR temp[MAX_PATH];
	GetModuleFileName(NULL, temp, MAX_PATH);
	PathCanonicalize(path, temp);
	PathQuoteSpaces(path);
}

bool CheckAutoRun()
{
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_QUERY_VALUE, &hKey)==ERROR_SUCCESS)
	{
		TCHAR buffer[MAX_PATH];
		DWORD dwLength = MAX_PATH;
		if(RegQueryValueEx(hKey, szClassName, NULL, NULL, (LPBYTE)buffer, &dwLength)==ERROR_SUCCESS)
		{
			RegCloseKey(hKey);

			TCHAR path[MAX_PATH];
			GetPrettyPath(path);

			if (_tcsicmp(path, buffer) == 0)
			{
				return true;
			}
		}
		RegCloseKey(hKey);
	}

	return false;
}

void SetAutoRun(HWND hwnd)
{
	HKEY hKey;
	RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey);
	if (!CheckAutoRun())
	{
		TCHAR path[MAX_PATH];
		GetPrettyPath(path);
		if (_tcsstr(path, _T("Temp")) != NULL)
		{
			if (MessageBox(hwnd, _T("您正在临时文件夹中运行本软件，不建议启用开机自动启动。\n您如果依然想要开机启动，请选择 是。"), szName, MB_YESNO + MB_ICONQUESTION + MB_DEFBUTTON2) == IDNO) return;
		}
		RegSetValueEx(hKey, szClassName, 0, REG_SZ, (LPBYTE)path, _tcslen(path)*sizeof(TCHAR));
	}
	else
	{
		RegDeleteValue(hKey, szClassName);
	}
	RegCloseKey(hKey);
}

void ShowContextMenu(lua_State *L, HMENU hMenu)
{
	if (lua_istable(L, -1))
	{
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			// -2=>key -1=>value
			if(lua_istable(L, -1) && lua_isnumber(L, -2) )
			{
				lua_pushstring(L, "name");
				lua_gettable(L, -2);
				const char* name = luaL_checkstring(L, -1);
				lua_pop(L, 1);

				lua_pushstring(L, "flag");
				lua_gettable(L, -2);
				int flag = lua_isnoneornil(L, -1)?0:luaL_checkint(L, -1);
				lua_pop(L, 1);

				lua_pushstring(L, "extend");
				lua_gettable(L, -2);
				if(lua_isfunction(L, -1))
				{
					int ref = luaL_ref(L, LUA_REGISTRYINDEX);
					lua_pushnil(L);
					AppendMenuW(hMenu, MF_BYPOSITION | MF_STRING | flag, ref, UnicodeFromUTF8(name));
				}
				else if(lua_istable(L, -1))
				{
					HMENU hPopMenu = CreatePopupMenu();
					ShowContextMenu(L, hPopMenu);
					AppendMenuW(hMenu, MF_POPUP | MF_BYPOSITION, (UINT)hPopMenu, UnicodeFromUTF8(name));
				}
				else
				{
					AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				}
				lua_pop(L, 1);
			}
			lua_pop(L, 1);
		}
	}
}

#define BUFSIZE 512
void DevicePathToWin32Path(wchar_t *strDevicePath)
{
	wchar_t szTemp[BUFSIZE];

	if (GetLogicalDriveStringsW(BUFSIZE-1, szTemp))
	{
		wchar_t szName[MAX_PATH];
		wchar_t szDrive[3] = L" :";
		wchar_t* p = szTemp;
		do
		{
			*szDrive = *p;
			if (QueryDosDeviceW(szDrive, szName, MAX_PATH))
			{
				UINT uNameLen = wcslen(szName);
				if (wcsnicmp(strDevicePath, szName, uNameLen)==0)
				{
					wchar_t szTempFile[MAX_PATH];
					wsprintfW(szTempFile, L"%s%s", szDrive, strDevicePath + uNameLen);
					wcscpy(strDevicePath, szTempFile);
				}
			}
			while (*p++);
		}
		while (*p);
	}
}

bool isEndWith(const char *path, const char* ext)
{
	if(!path || !ext) return false;
	int len1 = strlen(path);
	int len2 = strlen(ext);
	if(len2>len1) return false;
	return !_memicmp(path + len1 - len2, ext, len2*sizeof(char));
}

bool isEndWith(const wchar_t *path,const wchar_t* ext)
{
	int len1 = wcslen(path);
	int len2 = wcslen(ext);
	if(len2>len1) return false;
	return !memicmp(path + len1 - len2, ext, len2*sizeof(wchar_t));
}
