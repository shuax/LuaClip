--插件管理器

local plugins_instance = {}

--分发消息
function DispatchMessage(callback, message)
	for name,instance in pairs(plugins_instance) do
		local plugin_callback = instance[callback]
		if config.plugins[name] and plugin_callback then
			local ret = plugin_callback(message)
			if ret then return ret end --跳过下面处理
		end
	end
end

--插件注册
function RegisterPlugin(plugin)
	assert(plugin.name)
	--首次加入，默认禁用
	if config.plugins[plugin.name]==nil then
		config.plugins[plugin.name] = false
	end
	
	plugins_instance[plugin.name] = plugin
end

--尝试加载plugins目录下的插件
for _,path in pairs(ListPlugins()) do
	local ret, err = pcall(require, 'plugins.'..path)
	if not ret then
		print(err)
	end
end

--初始化所有插件
for name,enable in pairs(config.plugins) do
	if plugins_instance[name] then
		if enable and plugins_instance[name].Init then
			plugins_instance[name].Init()
		end
	else
		config.plugins[name] = nil
	end
end

--扩展配置文件操作
config.switch_plugin = function(name)
	config.plugins[name] = not config.plugins[name]
	if config.plugins[name] then
		if plugins_instance[name].Init then plugins_instance[name].Init() end
	else
		if plugins_instance[name].Uninit then plugins_instance[name].Uninit() end
	end
	save_config(config)
end

