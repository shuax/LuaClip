--全局变量
MAX_COUNT = 10
config = require('scripts.Config')

--引入文件
require('scripts.Helper.Helper')				--辅助文件
require('scripts.PluginManager')				--插件管理
require('scripts.OnRightButtonDown')			--托盘右键
require('scripts.OnClipboardChange')			--剪贴板变动
require('scripts.OnHotKeyDown')					--热键管理
require('scripts.Update')						--更新检查

--保存一次配置文件
save_config(config)
