local function InsertText(text)
	if #config.texts>=MAX_COUNT then
		--删除最后一条数据
		table.remove(config.texts)
	end

	if config.sort==1 then
		--最近添加
		table.insert(config.texts, 1, {text=text, add_time=os.time(), use_time=os.time()})
	elseif config.sort==2 then
		--最后使用
		table.insert(config.texts, {text=text, add_time=os.time(), use_time=os.time()})
	end
end

--剪贴板变化时调用，目前仅支持文本类型
function OnClipboardChange(text)

	if DispatchMessage('BeforeClipboardChange', text) then return end

	if config.duplicate then
		--允许重复数据
		InsertText(text)
	else
		--不允许重复数据
		local find_pos = nil
		for i,v in ipairs(config.texts) do
			if text==v.text then
				find_pos = i
				break
			end
		end

		if find_pos then
			--发现重复数据
			config.texts[find_pos].add_time = os.time()
		else
			InsertText(text)
		end
	end

	if config.sort==1 then
		--按照最近添加排序
		table.sort(config.texts, function(a,b) return a.add_time>b.add_time end)
	end

	if DispatchMessage('AfterClipboardChange', text) then return end

	save_config(config)
end