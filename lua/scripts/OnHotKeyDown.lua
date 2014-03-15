
local function OnHotKeyDown()
	local menu = GetHistory()
	
	AppendMenu(menu, "我是分割线")
	AppendMenu(menu, "清除记录(&C)", config.clear_texts)
	
	--在光标所在位置弹出菜单
	--弹出菜单会改变当前的激活窗口，因此SendText使用自动跟踪的窗口
	PopupMenu(menu)
end

--注册快捷菜单
RegisterHotKey("ctrl+alt+v", OnHotKeyDown)

--注册快捷使用按键，因为键盘排布关系，把10号分配为ctrl+0
--直接按下按键，没有弹出菜单，不会影响到当前激活窗口的焦点，无需使用自动更新的窗口
local function SendSerialText(i)
	if config.texts[i] then
		SendText(config.get_text(i), false)
	end
end

for i=1,9 do
	RegisterHotKey("ctrl+"..i, function() SendSerialText(i) end)
end

RegisterHotKey("ctrl+"..0, function() SendSerialText(10) end)
