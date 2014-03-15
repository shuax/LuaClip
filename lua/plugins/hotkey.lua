local plugin = {}

--基本信息
plugin.name = "热键关联"
plugin.author = "shuax"
plugin.description = "为你的任务安排一个热键。"
plugin.version = "1.0"

--内部数据
plugin.handles = {}

--初始化函数
plugin.Init = function()
	for line_ in io.lines("plugins\\hotkey.txt") do
		local line = line_:trim()
		local offset = string.find(line, ":")
		if not line:is_start_with("#") and offset then
			local path = line:sub(0, offset - 1)
			local hotkey = line:sub(offset + 1)
--			print(path, hotkey)

			local handle = RegisterHotKey(hotkey, function() Execute(path) end)
			table.insert(plugin.handles, handle)
		end
	end
end

--反初始化函数
plugin.Uninit = function()
	for _,handle in ipairs(plugin.handles) do
		UnregisterHotKey(handle)
	end
end

RegisterPlugin(plugin)
