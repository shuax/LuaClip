#include "resource.h"
#include "LuaClip.h"

//鼠标钩子函数
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode==HC_ACTION && wParam==WM_MOUSEWHEEL)
	{
		if(is_enabled && script.AllowWheelThrough())
		{
			MSLLHOOKSTRUCT *pmouse = (MSLLHOOKSTRUCT *)lParam;

			//获得按键和方向
			int fwKeys = GET_KEYSTATE_WPARAM(pmouse->mouseData);
			int zDelta = GET_WHEEL_DELTA_WPARAM(pmouse->mouseData);

			//修正按键
			if(GetAsyncKeyState(VK_SHIFT) & 0x8000) fwKeys |= MK_SHIFT;
			if(GetAsyncKeyState(VK_CONTROL) & 0x8000) fwKeys |= MK_CONTROL;
			if(GetAsyncKeyState(VK_LBUTTON) & 0x8000) fwKeys |= MK_LBUTTON;
			if(GetAsyncKeyState(VK_RBUTTON) & 0x8000) fwKeys |= MK_RBUTTON;
			if(GetAsyncKeyState(VK_MBUTTON) & 0x8000) fwKeys |= MK_MBUTTON;

			//发送消息，并且退出消息循环
			PostMessage(WindowFromPoint(pmouse->pt), wParam, MAKELONG(fwKeys, zDelta), MAKELPARAM(pmouse->pt.x, pmouse->pt.y));
			return 1;
		}
	}
	return CallNextHookEx(mouse_hook, nCode, wParam, lParam );
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static UINT const WM_TASKBARCREATED = RegisterWindowMessage( _T("TaskbarCreated") );
	static bool initialize = false;

	switch(message)
	{
	case WM_CREATE:
		tray.Init(hwnd, szName);
		script.Init(hwnd);
		SetClipboardViewer(hwnd);
		initialize = true;
		break;
	case WM_TRAYMESSAGE:
		switch(lParam)
		{
		case WM_LBUTTONUP:
			is_enabled = tray.SwitchIcon();
			break;
		case WM_RBUTTONDOWN:
			script.OnRightButtonDown(hwnd);
			break;
		default:
			if(WM_MOUSEFIRST<=lParam && lParam<=WM_MOUSELAST)
			{
			    //鼠标在托盘滑过时记录当前激活的窗口
				script.TrackActiveWnd();
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch(wParam)
		{
		case MENU_EXIT:
			DestroyWindow(hwnd);
			break;
		default:
			script.DispatchMessage(wParam);
		}
		break;
	case WM_HOTKEY:
		script.DispatchMessage(wParam);
		break;
	case WM_DRAWCLIPBOARD:
		if(is_enabled && !disable_once && initialize && CountClipboardFormats() && OpenClipboard(hwnd))
		{
			char* str = NULL;
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

			if(str)
			{
				if(str[0]) script.SendText(str); //非空字符串
				free(str);
			}
		}
		break;
	case WM_DESTROY:
		tray.Destory();
		PostQuitMessage(0);
		break;
	default:
		if(message == WM_TASKBARCREATED)//Explorer崩溃时重建图标
		{
			tray.Create();
			return 0;
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style	= CS_DBLCLKS;
	wcex.lpfnWndProc= (WNDPROC)WndProc;
	wcex.cbClsExtra	= 0;
	wcex.cbWndExtra	= 0;
	wcex.hInstance	= hInstance;
	wcex.hIcon	    = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
	wcex.hIconSm	= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON_SMALL));
	wcex.hCursor	= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szClassName;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hwnd = CreateWindowEx(0, szClassName, szName, WS_OVERLAPPEDWINDOW,
							   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, NULL, hInstance, NULL);

	if (!hwnd)
	{
		return FALSE;
	}

	ShowWindow(hwnd, SW_HIDE);

	return TRUE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//避免重复运行
	hMutex = CreateMutex(NULL, TRUE, szClassName);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex);
		return 0;
	}

	InitCommonControls();

	//注册程序类、窗口
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    SetCurrentDirectoryW(path);

    script.Load("scripts\\LuaClip.lua");

	mouse_hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);

	//消息循环
	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0))
	{
		TranslateMessage(&messages);
		DispatchMessage(&messages);
	}

	UnhookWindowsHookEx(mouse_hook);
	return messages.wParam;
}
