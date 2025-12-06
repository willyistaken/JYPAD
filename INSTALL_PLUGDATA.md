# XYPad 安裝到 PlugData 指南

## 快速安裝（推薦）

### macOS 安裝步驟

1. **編譯專案**（如果還沒編譯）
```bash
cd /Users/jinyaolin/plugins/build
cmake --build . --config Release
```

2. **安裝 VST3 插件**
```bash
# 複製到系統 VST3 目錄
cp -r build/PlugDataCustomObject_artefacts/VST3/XYPad.vst3 ~/Library/Audio/Plug-Ins/VST3/

# 或者安裝到全域目錄（需要管理員權限）
# sudo cp -r build/PlugDataCustomObject_artefacts/VST3/XYPad.vst3 /Library/Audio/Plug-Ins/VST3/
```

3. **安裝 AU 插件（可選）**
```bash
cp -r build/PlugDataCustomObject_artefacts/AU/XYPad.component ~/Library/Audio/Plug-Ins/Components/
```

4. **在 PlugData 中使用**

   **方法 A：如果 PlugData 支援載入 VST 插件**
   - 打開 PlugData
   - 使用 `[vstplugin~]` 或類似的物件來載入 XYPad.vst3
   - 或使用 PlugData 的插件瀏覽器（如果有）

   **方法 B：使用 Standalone 版本**
   - 直接運行 `build/PlugDataCustomObject_artefacts/Standalone/XYPad.app`
   - 通過 MIDI 或其他方式與 PlugData 通信

---

## 驗證安裝

### 檢查插件是否已安裝

```bash
# 檢查 VST3
ls -la ~/Library/Audio/Plug-Ins/VST3/XYPad.vst3

# 檢查 AU
ls -la ~/Library/Audio/Plug-Ins/Components/XYPad.component
```

### 測試插件

1. 打開任何支援 VST3 的 DAW（如 Reaper, Logic Pro, Ableton Live）
2. 載入 XYPad 插件
3. 確認 UI 正常顯示，可以拖動圓球

---

## 在 PlugData 中使用 XYPad

### 如果 PlugData 支援 VST 插件

1. 在 PlugData 中創建新物件
2. 輸入 `[vstplugin~ XYPad]` 或使用 PlugData 的插件載入功能
3. 連接輸出到其他物件

### 輸出格式

XYPad 會輸出 list 格式：`球編號 x y`

例如：
- `1 0.3 0.5` 表示 1 號球在 X=0.3, Y=0.5 的位置
- `2 -0.5 0.8` 表示 2 號球在 X=-0.5, Y=0.8 的位置

### 範例使用

```
[xypad] -> [list split] -> [其他處理物件]
```

---

## 開發 Pure Data 外部物件版本（進階）

如果您想要將 XYPad 作為 Pure Data 原生物件（`[xypad]`），需要：

1. **安裝 Pure Data 開發庫**
   - 下載 Pure Data 源代碼
   - 或使用 pd-lib-builder

2. **修改構建系統**
   - 添加 Pure Data 的包含路徑
   - 編譯為 `.pd_darwin`（macOS）、`.pd_linux`（Linux）或 `.pd_win`（Windows）

3. **安裝到 PlugData externals 目錄**
   ```bash
   # 找到 PlugData 的安裝目錄
   # 將編譯好的 .pd_darwin 文件複製到 externals 資料夾
   ```

**注意**：這需要 Pure Data 的開發環境，相對複雜。建議先使用 VST3 插件方式。

---

## 故障排除

### 插件無法載入

1. **檢查插件格式**
   ```bash
   file ~/Library/Audio/Plug-Ins/VST3/XYPad.vst3/Contents/MacOS/XYPad
   ```
   應該顯示：`Mach-O universal binary`

2. **檢查權限**
   ```bash
   chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/XYPad.vst3
   ```

3. **重新掃描插件**
   - 在 DAW 中重新掃描插件
   - 或重啟 DAW

### PlugData 找不到插件

- 確認 PlugData 支援載入外部 VST 插件
- 檢查 PlugData 的插件搜尋路徑設定
- 嘗試使用 Standalone 版本

---

## 下一步

如果您需要 Pure Data 原生物件版本，我可以幫您：
1. 設置 Pure Data 開發環境
2. 修改構建系統以編譯 .pd_darwin
3. 實現完整的 Pure Data 物件介面

請告訴我您想使用哪種方式！
