# 清除舊數據指南

如果插件因為舊數據格式問題無法打開，可以使用以下方法清除舊數據：

## 方法 1：在 DAW 中重置插件狀態（推薦）

### Reaper
1. 打開 Reaper
2. 找到使用 JYPad 的軌道
3. 右鍵點擊插件 → 選擇 "Reset to factory defaults" 或 "Clear plugin state"
4. 或者直接移除插件，然後重新添加

### Logic Pro
1. 打開 Logic Pro
2. 找到使用 JYPad 的軌道
3. 在插件窗口中，點擊插件名稱旁邊的菜單 → 選擇 "Reset" 或 "Initialize"
4. 或者移除插件後重新添加

### Ableton Live
1. 打開 Ableton Live
2. 找到使用 JYPad 的軌道
3. 右鍵點擊插件 → 選擇 "Reset" 或 "Remove Device"
4. 重新添加插件

## 方法 2：手動刪除插件狀態文件

### macOS (AU 插件)
插件狀態通常保存在：
```
~/Library/Preferences/[DAW名稱]/[插件狀態文件]
```

例如 Reaper：
```
~/Library/Application Support/REAPER/reaper-auplugins.ini
```

### 注意事項
- 刪除狀態文件可能會影響其他插件的設置
- 建議先備份相關文件
- 如果使用多個 DAW，需要在每個 DAW 中分別清除

## 方法 3：使用新版本自動處理

新版本已經添加了錯誤處理，當遇到舊格式數據時會：
1. 自動跳過無法讀取的舊數據
2. 使用默認設置啟動插件
3. 在 debug.log 中記錄錯誤信息

如果插件仍然無法打開，請檢查：
- `~/Documents/PlugDataCustomObject/debug.log` 文件
- 查看具體的錯誤信息

## 建議

如果遇到數據格式問題：
1. 先嘗試方法 1（在 DAW 中重置）
2. 如果還是不行，檢查 debug.log 文件
3. 最後才考慮手動刪除狀態文件


