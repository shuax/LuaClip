/*
	char str[] = ",,,1234,,,,,wt";
	StringSplit(str, ',', []
		(const char *str)
		{
			printf("1:%s\n", str);
		}
	);
*/
template<typename String, typename Char, typename Function>
void StringSplit(String *str, Char delim, Function f)
{
	String *ptr = str;
	while(*str)
	{
		if(*str==delim)
		{
			*str = 0;			//截断字符串

			if(str - ptr)		//非空字符串
			{
				f(ptr);
			}

			*str = delim;		//还原字符串
			ptr = str + 1;		//移动下次结果指针
		}
		str++;
	}

	if(str-ptr)	 //非空字符串
	{
		f(ptr);
	}
}


bool IsSystemWin7()
{
	static int result = -1;
	if(result==-1)
	{
		OSVERSIONINFO osvi;
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&osvi);
		
		if (osvi.dwMajorVersion > 6 )
		{
			result = true;
		}
		else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 1 )
		{
			result = true;
		}
		else
		{
			result = false;
		}
	}
	return result;
}

UINT ParseHotkeys(const char* keys)
{
	UINT mo = 0;
	UINT vk = 0;

	char *temp = strdup(keys);
	StringSplit(temp, '+', [&mo,&vk]
		(char *key)
		{
			//解析控制键
			if(stricmp(key,"Shift")==0) mo |= MOD_SHIFT;
			else if(stricmp(key,"Ctrl")==0) mo |= MOD_CONTROL;
			else if(stricmp(key,"Alt")==0) mo |= MOD_ALT;
			else if(stricmp(key,"Win")==0) mo |= MOD_WIN;

			char wch = key[0];
			if (strlen(key)==1)			//解析单个字符A-Z、0-9等
			{
				if(isalnum(wch)) vk = toupper(wch);
				else vk = LOWORD(VkKeyScan(wch));
			}
			else if (wch=='F'||wch=='f')		//解析F1-F24功能键
			{
				if(isdigit(key[1]))
				{
					int FX = atoi(&key[1]);
					if(FX>=1&&FX<=24) vk = VK_F1 + FX - 1;
				}
			}
			else								// 解析其他按键
			{
				if(stricmp(key,"Left")==0) vk = VK_LEFT;
				else if(stricmp(key,"Right")==0) vk = VK_RIGHT;
				else if(stricmp(key,"Up")==0) vk = VK_UP;
				else if(stricmp(key,"Down")==0) vk = VK_DOWN;

				else if(stricmp(key,"←")==0) vk = VK_LEFT;
				else if(stricmp(key,"→")==0) vk = VK_RIGHT;
				else if(stricmp(key,"↑")==0) vk = VK_UP;
				else if(stricmp(key,"↓")==0) vk = VK_DOWN;

				else if(stricmp(key,"Esc")==0) vk = VK_ESCAPE;
				else if(stricmp(key,"Tab")==0) vk = VK_TAB;

				else if(stricmp(key,"Backspace")==0) vk = VK_BACK;
				else if(stricmp(key,"Enter")==0) vk = VK_RETURN;
				else if(stricmp(key,"Space")==0) vk = VK_SPACE;

				else if(stricmp(key,"PrtSc")==0) vk = VK_SNAPSHOT;
				else if(stricmp(key,"Scroll")==0) vk = VK_SCROLL;
				else if(stricmp(key,"Pause")==0) vk = VK_PAUSE;

				else if(stricmp(key,"Insert")==0) vk = VK_INSERT;
				else if(stricmp(key,"Delete")==0) vk = VK_DELETE;

				else if(stricmp(key,"End")==0) vk = VK_END;
				else if(stricmp(key,"Home")==0) vk = VK_HOME;

				else if(stricmp(key,"PageUp")==0) vk = VK_PRIOR;
				else if(stricmp(key,"PageDown")==0) vk = VK_NEXT;
			}

		}
	);

	free(temp);

	if( IsSystemWin7() ) mo |= 0x4000;

	return MAKELPARAM(mo, vk);
}
