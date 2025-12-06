# JYPad v0.1.0

JYPad 是一個功能強大的 2D 控制器插件，專為 PlugData 和現代 DAW 設計。它提供了一個可視化的 XY 平面，可以同時控制多個可拖動的球體，並支援 MIDI 錄製、OSC 通信等功能。

## 功能特色

- **多球控制**：在 2D 平面上同時控制多個可拖動的球體
- **MIDI 錄製與回放**：錄製球體的移動軌跡，並根據 MIDI 時間自動回放
- **OSC 通信**：支援 OSC 協議，可自定義 OSC 前綴
- **時間碼同步**：自動同步 DAW 的時間碼和 MIDI 時間
- **狀態管理**：支援 Mute、Solo、Recording 等狀態
- **數據持久化**：所有設定和錄製數據會自動保存，重啟後自動載入

## 系統需求

- **macOS**: 10.13 或更高版本
- **DAW**: 支援 VST3 或 AU 格式的 DAW（如 Reaper, Logic Pro, Ableton Live 等）
- **PlugData**: 支援載入 VST 插件的版本

## 安裝說明

### macOS 安裝步驟

1. **編譯專案**（如果還沒編譯）
```bash
cd /path/to/plugins/build
cmake --build . --config Release
```

2. **安裝 VST3 插件**
```bash
# 複製到用戶 VST3 目錄
cp -r build/PlugDataCustomObject_artefacts/VST3/JYPad.vst3 ~/Library/Audio/Plug-Ins/VST3/

# 或者安裝到系統目錄（需要管理員權限）
# sudo cp -r build/PlugDataCustomObject_artefacts/VST3/JYPad.vst3 /Library/Audio/Plug-Ins/VST3/
```

3. **安裝 AU 插件（可選）**
```bash
cp -r build/PlugDataCustomObject_artefacts/AU/JYPad.component ~/Library/Audio/Plug-Ins/Components/
```

4. **驗證安裝**
```bash
# 檢查 VST3
ls -la ~/Library/Audio/Plug-Ins/VST3/JYPad.vst3

# 檢查 AU
ls -la ~/Library/Audio/Plug-Ins/Components/JYPad.component
```

5. **在 DAW 中載入**
   - 打開您的 DAW（如 Reaper, Logic Pro 等）
   - 重新掃描插件（如果需要）
   - 在插件列表中尋找 "JYPad"
   - 載入插件即可使用

## 使用說明

### 基本操作

1. **拖動球體**：在 XY 平面上點擊並拖動球體來改變其位置
2. **添加球體**：在空白處右鍵點擊，選擇 "Add Source" 來添加新球體
3. **編輯球體**：在球體上右鍵點擊，選擇 "Edit" 來修改球體的屬性

### 球體屬性設定

每個球體可以設定以下屬性：
- **OSC Prefix**：OSC 訊息的前綴（例如：`/jypad/ball1`）
- **Color**：球體的顯示顏色
- **Source Name**：球體的名稱
- **Source Number**：球體的編號

### 狀態控制

每個球體支援以下狀態：
- **Mute**：靜音狀態，球體變為半透明且無法移動
- **Solo**：獨奏狀態，球體會顯示黃色標記
- **Recording**：錄製狀態，球體會顯示閃爍的紅圈

### MIDI 錄製與回放

1. **開始錄製**：
   - 在球體上右鍵，選擇 "Recording" 來啟用錄製狀態
   - 拖動球體，系統會自動記錄位置和 MIDI 時間

2. **回放錄製**：
   - 在 DAW 中播放，球體會根據錄製的數據自動移動
   - 只有非錄製狀態的球體會自動回放

3. **管理錄製數據**：
   - **Clear Events**：清除該球體的所有錄製數據
   - **Set Position**：在當前時間點插入當前位置作為事件
   - **Tween to next**：在當前位置和下一個事件之間生成插值事件

### OSC 通信

1. **設定 OSC**：
   - 點擊右上角的 "NETWORK" 按鈕
   - 設定 OSC 發送 IP 地址和端口（默認：127.0.0.1:4002）
   - 啟用 OSC 功能

2. **OSC 訊息格式**：
   - 地址：`{oscPrefix}`（例如：`/jypad/ball1`）
   - 參數：`x y z`（三個浮點數，z 目前為 0）

3. **測試 OSC**：
   - 在 Network Settings 視窗中點擊 "Test" 按鈕來測試 OSC 連接

### 時間碼顯示

插件左上角會顯示：
- **MIDI Time**：格式為 `Bar.Beat.Ticks`（例如：`1.2.480`）
- **Time Code**：格式為 `m:ss.SSS`（例如：`0:15.234`）
- **BPM 資訊**：顯示當前 BPM 和播放狀態

### 數據管理

1. **OSC Data 視窗**：
   - 點擊右上角的 "OSC DATA" 按鈕
   - 查看和編輯所有球體的數據表格

2. **數據持久化**：
   - 所有設定、球體位置、錄製數據都會自動保存
   - 下次打開插件時會自動載入

## 在 PlugData 中使用

### 方法 A：使用 VST 插件

如果 PlugData 支援載入 VST 插件：
1. 在 PlugData 中使用 `[vstplugin~]` 或類似的物件來載入 JYPad.vst3
2. 連接輸出到其他物件進行處理

### 方法 B：使用 Standalone 版本

1. 直接運行 `build/PlugDataCustomObject_artefacts/Standalone/JYPad.app`
2. 通過 MIDI 或 OSC 與 PlugData 通信

## 故障排除

### 插件無法載入

1. **檢查插件格式**：
   ```bash
   file ~/Library/Audio/Plug-Ins/VST3/JYPad.vst3/Contents/MacOS/JYPad
   ```
   應該顯示：`Mach-O universal binary`

2. **檢查權限**：
   ```bash
   chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/JYPad.vst3
   ```

3. **重新掃描插件**：
   - 在 DAW 中重新掃描插件
   - 或重啟 DAW

### OSC 無法連接

1. 檢查 IP 地址和端口是否正確
2. 確認接收端（如 PlugData）是否正在監聽該端口
3. 檢查防火牆設定

### 錄製數據丟失

- 確保在關閉 DAW 前保存專案
- 插件狀態會自動保存到 DAW 的專案文件中

## 版本資訊

- **版本**：v0.1.0
- **構建日期**：2024
- **授權**：請參考 LICENSE 文件

## 技術支援

如有問題或建議，請聯繫開發團隊。

---

**JYPad v0.1.0** - 強大的 2D 控制器插件

