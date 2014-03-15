--读取配置文件
local config = {}
if pcall(require, 'settings') then
	config = require('settings')
	if type(config)~='table' then config = {} end
end

--配置文件初始化
config.texts = config.texts or {}
config.plugins = config.plugins or {}
config.sort = config.sort or 1				--1最近添加 2最近使用
if config.duplicate==nil then config.duplicate = false end	--false不允许重复 true允许重复

config.clear_texts = function()
	config.texts = {}
	save_config(config)
end

config.get_text = function(i)
	local text = config.texts[i].text
	config.texts[i].use_time = os.time()
	if config.sort==2 then
		--按照最近使用排序
		table.sort(config.texts, function(a,b) return a.use_time>b.use_time end)
	end
	save_config(config)
	return text
end

config.set = function(key, value)
	config[key] = value
	save_config(config)
end

--保存配置文件
function save_config(settings, file_path)
	file_path = file_path or "settings.lua"
	local function serialize(t)
		if type(t)~="table" then return tostring(t) end --
		local function ser_table(tbl,level)
			level = level or 1
			local align = "\n" .. string.rep("\t", level-1)
			local indent = string.rep("\t", level)

			local tmp = {}
			for k,v in pairs(tbl) do
				local key = type(k)=="number" and "["..k.."]" or string.format("[%q]", k)
				if type(v)=="table" then
					table.insert(tmp, indent..key.." = "..ser_table(v, level + 1))
				else
					if type(v)=="string" then
						table.insert(tmp, indent..key..string.format("=%q",v))
					elseif type(v)=="boolean" then
						table.insert(tmp, indent..key.." = ".. (v and "true" or "false") )
					elseif type(v)=="number" then
						table.insert(tmp, indent..key.." = "..v)
					end
				end
			end
			return align .. "{\n" .. table.concat(tmp,",\n") .. align .. "}"
		end

		return "--这是LuaClip的配置文件\ndo\nlocal settings = " .. ser_table(t) .. "\nreturn settings\nend"
	end
	
	local file = io.open(file_path, "w")
	if file then
		file:write(serialize(settings))
		file:close()
	end
end

return config