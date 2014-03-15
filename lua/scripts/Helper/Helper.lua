--一些辅助函数

--添加一个项目到主菜单；name为名字；extend为回调函数或者子菜单，否则就是分割线；flag为菜单参数
function AppendMenu(t, name, extend, flag)
	table.insert(t, {name=name, extend=extend, flag=flag})
end

--获取历史记录
function GetHistory()
	local menu = {}
	if #config.texts==0 then
		AppendMenu(menu, "(暂无记录)", function() end, MF_GRAYED + MF_DISABLED)
	else
		for i,v in ipairs(config.texts) do
			if i==10 then
				AppendMenu(menu, "1&0. "..v.text:sub_utf8(24), function() SendText(config.get_text(i))end)
			else
				AppendMenu(menu, "&".. i ..". "..v.text:sub_utf8(24), function() SendText(config.get_text(i)) end)
			end
		end
	end
	return menu
end

--其它辅助功能
require('scripts.Helper.const')			--常量定义
require('scripts.Helper.string')		--字符串扩展

