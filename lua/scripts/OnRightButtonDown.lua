--右键点击托盘图标时调用
function OnRightButtonDown()
	local menu = {}

	--构造历史记录菜单
	AppendMenu(menu, "历史记录(&H)", GetHistory(true))

	local mgr_menu = {}
		local sub_menu = {}
		AppendMenu(sub_menu, "最近添加(&U)", function() config.set("sort", 1) end, config.sort==1 and MF_CHECKED + MFT_RADIOCHECK or 0)
		AppendMenu(sub_menu, "最近使用(&A)", function() config.set("sort", 2) end, config.sort==2 and MF_CHECKED + MFT_RADIOCHECK or 0)
		AppendMenu(mgr_menu, "排序方式(&S)", sub_menu)

		AppendMenu(mgr_menu, "我是分割线")
		AppendMenu(mgr_menu, "自动去重(&D)", function() config.set("duplicate", not config.duplicate and true or false) end, not config.duplicate and MF_CHECKED or 0)
		AppendMenu(mgr_menu, "清除记录(&C)", config.clear_texts)

	AppendMenu(menu, "记录管理(&M)", mgr_menu)
	AppendMenu(menu, "我是分割线")


	--构造插件列表
	sub_menu = {}
	for name,enable in pairs(config.plugins) do
		AppendMenu(sub_menu, name, function() config.switch_plugin(name) end, enable and MF_CHECKED or 0)
	end
	AppendMenu(menu, "插件管理(&P)", sub_menu)

	--构造程序设置
	sub_menu = {}
	AppendMenu(sub_menu, "开机启动(&S)", SetAutoRun, CheckAutoRun() and MF_CHECKED or 0)
	AppendMenu(sub_menu, "重启程序(&R)", Restart)
	AppendMenu(sub_menu, "关于我们(&A)", function()
		MessageBox("欢迎使用我的剪贴板增强软件，使用Lua扩展逻辑。\n\nhttp://shuax.com", "关于 LuaClip", MB_ICONINFORMATION + MB_OK)
	end)
	AppendMenu(menu, "程序设置(&L)", sub_menu)

	AppendMenu(menu, "我是分割线")

	AppendMenu(menu, "退出程序(&E)", ExitProcess)

	--返回弹出菜单项目
	return menu
end
