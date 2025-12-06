# 中文字體設置說明

插件現在支持中文字體顯示。字體管理器會自動嘗試載入系統中已安裝的中文字體。

## 自動載入的系統字體

### macOS
插件會按順序嘗試載入以下字體：
1. **PingFang SC** - macOS 系統默認中文字體（推薦）
2. STHeiti - 黑體
3. STSong - 宋體
4. Hiragino Sans GB - 冬青黑體
5. Arial Unicode MS
6. Noto Sans CJK SC（如果已安裝）
7. Source Han Sans SC（如果已安裝）

### Windows
1. **Microsoft YaHei** - 微軟雅黑（Windows 系統默認）
2. SimHei - 黑體
3. SimSun - 宋體
4. KaiTi - 楷體
5. Noto Sans CJK SC（如果已安裝）
6. Source Han Sans SC（如果已安裝）

### Linux
1. Noto Sans CJK SC
2. Source Han Sans SC
3. WenQuanYi Micro Hei - 文泉驛微米黑
4. WenQuanYi Zen Hei - 文泉驛正黑
5. AR PL UMing CN

## 如果系統沒有中文字體

如果您的系統沒有安裝中文字體，可以下載並安裝開源字體：

### 推薦：Noto Sans CJK（思源黑體）

**下載地址：**
- Google Fonts: https://fonts.google.com/noto/specimen/Noto+Sans+SC
- GitHub: https://github.com/googlefonts/noto-cjk

**安裝方法：**

#### macOS
1. 下載 Noto Sans CJK 字體文件（.ttf 或 .otf）
2. 雙擊字體文件
3. 點擊「安裝字體」
4. 重新啟動插件

#### Windows
1. 下載 Noto Sans CJK 字體文件
2. 右鍵點擊字體文件
3. 選擇「安裝」
4. 重新啟動插件

#### Linux
```bash
# 使用包管理器安裝
sudo apt-get install fonts-noto-cjk  # Debian/Ubuntu
sudo yum install google-noto-cjk-fonts  # CentOS/RHEL
```

## 驗證字體是否載入

插件會自動檢測並載入可用的中文字體。如果中文文字正常顯示，說明字體已成功載入。

## 嵌入字體到插件（進階）

如果您想將字體直接嵌入到插件中（不依賴系統字體），可以：

1. 下載字體文件（.ttf 或 .otf）
2. 將字體文件添加到專案資源中
3. 修改 `FontManager.cpp` 中的 `loadEmbeddedFont()` 方法
4. 使用 `juce::Typeface::createSystemTypefaceFor()` 載入字體

範例代碼已在 `FontManager.cpp` 中提供（註釋掉的範例）。

## 故障排除

### 中文仍然無法顯示

1. **檢查系統是否安裝了中文字體**
   - macOS: 打開「字體冊」應用，搜索「PingFang」或「STHeiti」
   - Windows: 打開「字體」設置，搜索「Microsoft YaHei」
   - Linux: 運行 `fc-list :lang=zh`

2. **重新編譯插件**
   ```bash
   cd build
   cmake --build . --config Release
   ```

3. **重新啟動 DAW**
   - 完全關閉並重新打開 DAW
   - 重新載入插件

4. **檢查字體權限**
   - 確保字體文件有讀取權限

## 技術細節

字體管理器使用單例模式，在插件啟動時自動初始化。它會：
1. 嘗試載入系統中文字體
2. 測試字體是否能正確顯示中文（使用「測試」字樣）
3. 如果成功，使用該字體；否則使用默認字體

所有 UI 元件（Label、TextEditor、Graphics 等）都會使用 `FontManager::getInstance().getChineseFont()` 來獲取中文字體。


