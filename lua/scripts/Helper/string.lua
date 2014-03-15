--字符串函数扩展

string.trim = function(str)
	return str:match'^%s*(.*%S)' or ''
end

string.explode = function(str, sep)
	local function DontAddSpace(t, s)
		if #s>0 then table.insert(t, s) end
	end
	local tble = {}
	local ll = 0
	
	while (true) do
		local l = string.find(str, sep, ll, true)
		if l then
			DontAddSpace(tble, string.sub(str, ll, l - 1)) 
			ll = l + 1
		else
			DontAddSpace(tble, string.sub(str, ll))
			break
		end
		
	end
	
	return tble
end

string.replace = function(str, src, rep)
	local start = 1
	while (true) do
		local pos = string.find(str, src, start, true)
	
		if (pos == nil) then
			break
		end
		
		local left = string.sub(str, 1, pos-1)
		local right = string.sub(str, pos + #src)
		
		str = left .. rep .. right
		start = pos + #rep
	end
	return str
end

string.is_start_with = function(str, substr)
	return str:sub(0, #substr):lower()==substr:lower()
end

string.is_end_with = function(str, substr)
	return str:sub(#str - #substr + 1):lower()==substr:lower()
end


--UTF8字符串处理

--根据首字节获取UTF8一个字符需要的字节数
local function GetUTF8NextChar(ch)
	local utf8_skip_table = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 1, 1,
	}
	return utf8_skip_table[ch]
end

--根据UTF8流获取字符串长度
--GetUTF8Length("一二三四五六七") 返回7
local function GetUTF8Length(str)
	local len = 0
	local ptr = 1
	repeat
		local char = string.byte(str, ptr)
		ptr = ptr + GetUTF8NextChar(char)
		len = len + 1
	until(ptr>#str)
	return len
end

--截取UTF8字符串
--SubUTF8String("一二三四五六七",1,3) 返回一二三
local function SubUTF8String(str, begin, length)
	begin = begin or 1
	length = length or -1 --length为-1时代表不限制长度
	local ret = ""
	local len = 0
	local ptr = 1
	repeat
		local char = string.byte(str, ptr)
		local char_len = GetUTF8NextChar(char)
		len = len + 1

		if len>=begin and (length==-1 or len<begin+length) then
			for i=0,char_len-1 do
				ret = ret .. string.char( string.byte(str, ptr + i) )
			end
		end

		ptr = ptr + char_len
	until(ptr>#str)
	return ret
end

string.sub_utf8 = function(str, length)
	if #str==0 then return str end
	return SubUTF8String(str, 1, length)
end