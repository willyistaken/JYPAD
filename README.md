# XYPad - PlugData 自訂物件

這是一個使用 JUCE 框架開發的 PlugData 自訂物件，實現了一個 2D XY Pad 控制器，可以顯示和控制多個可拖動的圓球。

## 功能特點

- **2D XY Pad 控制器**：在 2D 平面上顯示和控制多個圓球
- **座標系統**：中心點為 (0, 0)，座標範圍為 -1.0 到 1.0
- **多球體支援**：可以添加和管理多個圓球，每個球都有唯一的編號
- **即時輸出**：移動圓球時會輸出格式化的字串，例如：`1 0.3 0.5`（球編號 x座標 y座標）
- **視覺化 UI**：使用 JUCE 繪製的現代化界面，包含網格和彩色圓球
- **狀態儲存**：支援儲存和載入圓球位置

## 專案結構

```
plugins/
├── CMakeLists.txt          # CMake 構建配置
├── Source/
│   ├── PluginProcessor.h   # 音訊處理器標頭檔
│   ├── PluginProcessor.cpp # 音訊處理器實作
│   ├── PluginEditor.h     # UI 編輯器標頭檔
│   ├── PluginEditor.cpp   # UI 編輯器實作
│   ├── XYPad.h            # XYPad 物件標頭檔
│   ├── XYPad.cpp          # XYPad 物件實作
│   ├── XYPadEditor.h      # XYPad 視覺化編輯器標頭檔
│   └── XYPadEditor.cpp    # XYPad 視覺化編輯器實作
└── README.md              # 本文件
```

## 前置需求

1. **JUCE 框架**
   - 下載 JUCE 並放置在專案目錄中，或使用 git submodule
   - 建議版本：JUCE 7.0 或更新版本

2. **CMake** (3.22 或更新版本)

3. **編譯器**
   - macOS: Xcode (最新版本)
   - Windows: Visual Studio 2019 或更新版本
   - Linux: GCC 或 Clang

## 設定專案

### 1. 下載 JUCE

```bash
# 選項 1: 使用 git submodule
cd /Users/jinyaolin/plugins
git submodule add https://github.com/juce-framework/JUCE.git JUCE

# 選項 2: 手動下載並解壓到專案目錄
# 確保 JUCE 目錄位於專案根目錄
```

### 2. 生成構建文件

```bash
# 創建構建目錄
mkdir build
cd build

# 生成專案文件
cmake ..

# macOS: 使用 Xcode
open PlugDataCustomObject.xcodeproj

# 或直接編譯
cmake --build . --config Release
```

## 使用說明

### 座標系統

- **中心點**：XY Pad 的中心為 (0, 0)
- **座標範圍**：X 和 Y 軸的範圍都是 -1.0 到 1.0
- **輸出格式**：當圓球移動時，會輸出格式為 `球編號 x座標 y座標` 的字串
  - 例如：`1 0.3 0.5` 表示 1 號球在 X=0.3, Y=0.5 的位置

### 操作方式

1. **拖動圓球**：點擊並拖動圓球來改變其位置
2. **點擊空白處**：點擊空白區域會移動第一個圓球到該位置
3. **即時輸出**：移動圓球時，輸出區域會即時顯示當前位置資訊

### 程式化使用

```cpp
// 添加新球
xyPad.addBall(1, 0.0f, 0.0f);  // 在中心添加 1 號球
xyPad.addBall(2, 0.5f, -0.3f); // 添加 2 號球

// 設定球的位置
xyPad.setBallPosition(1, 0.3f, 0.5f);

// 獲取輸出字串
juce::String output = xyPad.getBallOutputString(1);  // 返回 "1 0.300 0.500"

// 獲取所有球
const auto& balls = xyPad.getAllBalls();
for (const auto& ball : balls) {
    // 處理每個球
}
```

## 自訂開發

### 修改 UI 外觀

在 `Source/XYPadEditor.cpp` 中，您可以：
- 修改 `ballRadius` 來改變圓球大小
- 在 `drawGrid()` 中自訂網格樣式
- 在 `drawBalls()` 中自訂圓球顏色和樣式

### 添加功能

- **添加更多圓球**：在 `XYPad` 類別中添加 `addBall()` 方法調用
- **自訂輸出格式**：修改 `getBallOutputString()` 方法
- **添加音訊處理**：在 `XYPad::processBlock()` 中實現 DSP 邏輯

## 編譯輸出

編譯完成後，插件將生成在：
- **macOS**: `build/PlugDataCustomObject_artefacts/Release/AU/` 或 `VST3/`
- **Windows**: `build/PlugDataCustomObject_artefacts/Release/VST3/`
- **Linux**: `build/PlugDataCustomObject_artefacts/Release/LV2/` 或 `VST3/`

## 在 PlugData 中使用

1. 將編譯好的插件安裝到系統的插件目錄
2. 在 PlugData 中創建新的物件
3. 使用 XYPad 物件進行 2D 控制
4. 連接輸出到其他物件來處理座標數據

## 技術細節

- **座標轉換**：UI 座標 (0-1) 與邏輯座標 (-1 到 1) 之間的自動轉換
- **回調機制**：使用 `std::function` 實現球移動時的回調
- **狀態管理**：使用 JUCE 的 `MemoryOutputStream` 和 `MemoryInputStream` 進行狀態儲存

## 授權

請根據您的需求設定適當的授權條款。
