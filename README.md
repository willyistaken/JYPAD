# JYPad - PlugData 自訂物件 (V0.3.0)

這是一個使用 JUCE 框架開發的 PlugData 自訂物件，實現了一個 2D Pad 控制器，可以顯示和控制多個可拖動的圓球，並支援 OSC 訊號傳輸與自動化錄製。

## 版本資訊 V0.3.0 (2025-12)

**主要更新：效能與穩定性優化**

1.  **記憶體優化**
    *   修復了錄製時資料重複寫入兩次的問題，減少 50% 記憶體佔用。
    *   錄製時完全暫存於記憶體，僅在存檔時寫入磁碟，確保最高錄製效能。

2.  **效能優化 (CPU & UI)**
    *   **搜尋演算法升級**：將錄製資料的搜尋從線性搜尋 (O(N)) 升級為二元搜尋 (O(log N))，大幅提升大量資料回放時的效率。
    *   **UI 渲染優化**：移除了計時器中的強制重繪 (Force Repaint)，改為事件驅動 (Event-Driven) 更新。只有當球體實際移動時才更新畫面。
    *   **冗餘檢查**：加入 Epsilon 檢查，當滑鼠微小移動或數值未變動時，自動忽略處理，節省 OSC 頻寬與 CPU 資源。

3.  **穩定性修復**
    *   **網路不卡頓**：移除了 OSC 發送失敗時的同步重連邏輯，解決了網路斷線會導致音訊/UI 卡死的問題。
    *   **執行緒安全**：改用 `std::atomic` 處理跨執行緒資料，移除潛在的 Deadlock 風險。
    *   **UI 修正**：修復了 Replay 時 UI 不會更新的 Bug。
    *   **在地化**：將「清除事件」等確認視窗中文化/英文化修正。

## 功能特點

- **2D Pad 控制器**：在 2D 平面上顯示和控制多個圓球
- **座標系統**：中心點為 (0, 0)，座標範圍為 -1.0 到 1.0
- **多球體支援**：可以添加和管理多個圓球，每個球都有唯一的編號
- **OSC 傳輸**：支援自動發送 OSC 訊號 (/track/n/x, /track/n/y)
- **自動化錄製**：內建記憶體錄製功能，可記錄並回放球體移動軌跡
- **視覺化 UI**：使用 JUCE 繪製的現代化界面，包含網格、殘影與閃爍指示
- **狀態儲存**：支援儲存和載入所有球體與錄製資料到 DAW 專案中

## 專案結構

```
plugins/
├── CMakeLists.txt          # CMake 構建配置
├── Source/
│   ├── PluginProcessor.h   # 音訊處理器標頭檔
│   ├── PluginProcessor.cpp # 音訊處理器實作
│   ├── PluginEditor.h     # UI 編輯器標頭檔
│   ├── PluginEditor.cpp   # UI 編輯器實作
│   ├── JYPad.h            # JYPad 物件標頭檔 (核心邏輯)
│   ├── JYPad.cpp          # JYPad 物件實作
│   ├── JYPadEditor.h      # JYPad 視覺化編輯器標頭檔
│   └── JYPadEditor.cpp    # JYPad 視覺化編輯器實作
└── README.md              # 本文件
```

## 前置需求

1. **JUCE 框架** (推薦 JUCE 7+)
2. **CMake** (3.22+)
3. **C++ 編譯器** (Xcode/Visual Studio/GCC)

## 建置與安裝

使用我們提供的安裝腳本即可自動編譯並安裝：

```bash
cd /Users/jinyaolin/plugins
sh install.sh
```

手動編譯：

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## 使用說明

### 座標與操作
- **中心點**：(0, 0)
- **範圍**：-1.0 到 1.0
- **基本操作**：
    - **左鍵拖曳**：移動球體 (若在錄製模式則會記錄軌跡)
    - **雙擊**：切換球體的 Recording 狀態
    - **右鍵**：開啟右鍵選單 (Edit, Delete, Mute, Solo, Clear Events)

### 程式化接口 (C++)

```cpp
// 範例：在 ProcessBlock 中更新
jyPad.processBlock(buffer, midiMessages);

// 獲取球體並設定位置
jyPad.setBallPosition(1, 0.5f, 0.5f);

// 錄製事件 (直接寫入記憶體)
jyPad.recordEvent(1, midiTime, x, y, z);
```

## 編譯輸出位置
- **macOS (AU)**: `~/Library/Audio/Plug-Ins/Components/JYPad.component`
- **macOS (VST3)**: `~/Library/Audio/Plug-Ins/VST3/JYPad.vst3`

## 授權
Private / Custom Implementation for PlugData.

