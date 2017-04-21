local M = {}
M.description_to_filename = {}
M.description_to_loaded_func = {}
M.descriptions = {}

M.descriptions[1] = "What do you want?"
M.description_to_filename[ [==[在Kindle书店里搜索你想要找的书]==] ] = 'SearchKindle.lua'
M.descriptions[2] = [==[在Kindle书店里搜索你想要找的书]==]
M.description_to_filename[ [==[在微博上关注小扳手作者包昊军（不一定成功）]==] ] = 'follow-me.lua'
M.descriptions[3] = [==[在微博上关注小扳手作者包昊军（不一定成功）]==]
M.description_to_filename[ [==[打开京东 App，扫描登录 PC 网页]==] ] = 'scan_jd.lua'
M.descriptions[4] = [==[打开京东 App，扫描登录 PC 网页]==]
M.description_to_filename[ [==[通过微博、微信朋友圈传播小扳手（不一定成功）]==] ] = 'spread-it.lua'
M.descriptions[5] = [==[通过微博、微信朋友圈传播小扳手（不一定成功）]==]
M.description_to_filename[ [==[打开通知授权设置页，再手动（可能要多次尝试）打开、关闭小扳手接收通知权限]==] ] = 'switch-notification.lua'
M.descriptions[6] = [==[打开通知授权设置页，再手动（可能要多次尝试）打开、关闭小扳手接收通知权限]==]
M.description_to_filename[ [==[打开微博扫码功能]==] ] = 'weibo_scan.lua'
M.descriptions[7] = [==[打开微博扫码功能]==]
return M