#include "resource.h"

#define WM_TRAYMESSAGE (WM_USER + 0x100)

class TrayIcon
{
public:
	void Init(HWND hwnd,TCHAR *name)
	{
		activate = false;

		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = hwnd;
		nid.uID = 100;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = WM_TRAYMESSAGE;
		_tcscpy(nid.szTip, name);
		Create();

		SwitchIcon();
	}

	bool Create()
	{
		return Shell_NotifyIcon(NIM_ADD, &nid);
	}

	bool SwitchIcon()
	{
		if(!activate)
		{
			activate = true;
			nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON_SMALL));
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}
		else
		{
			activate = false;
			nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_APPICON_DISABLED));
			Shell_NotifyIcon(NIM_MODIFY, &nid);
		}

		return activate;
	}

	bool Destory()
	{
		return Shell_NotifyIcon(NIM_DELETE, &nid);
	}
private:
	NOTIFYICONDATA nid;
	bool activate;
};

