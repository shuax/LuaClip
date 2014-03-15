local function version_format(number)
	local str = ""
	repeat
		local mantissa = number%10
		number = (number - mantissa)/10
		
		if number~=0 then
			str = "." .. mantissa .. str
		else
			str = mantissa .. str
			break
		end
	until false
	return str
end

--检查更新
local function check_update(str)
	Sleep(3*1000)
	local ver = tonumber(str)
	if ver and LUACLIP_VERSION<ver then
		local tips = string.format('发现LuaClip新版本"%s"，您现在使用的版本是"%s"。\n\n是否立刻查看更新？', version_format(ver), version_format(LUACLIP_VERSION))
		if MessageBox(tips, "更新提示", MB_ICONQUESTION + MB_YESNO)==IDYES then
			ShellExecute("http://www.shuax.com/archives/LuaClip.html")
		end
	end
end
CreateThread(Http, check_update, "http://www.shuax.com/update/luaclip")