# AAX 格式編譯指南

## 概述

JUCE 支援編譯 AAX 格式插件，這是 Avid Pro Tools 使用的插件格式。

## 要求

1. **AAX SDK**：✅ JUCE 已經包含了 AAX SDK（在 `JUCE/modules/juce_audio_plugin_client/AAX/SDK/`），**無需額外下載**
2. **macOS**：AAX 插件可以在 macOS 上編譯（Windows 版本需要 Windows 版本的 AAX SDK）
3. **Pro Tools**：用於測試 AAX 插件（可選）

## 啟用 AAX 格式

### 步驟 1：修改 CMakeLists.txt

在 `juce_add_plugin` 的 `FORMATS` 參數中添加 `AAX`：

```cmake
juce_add_plugin(PlugDataCustomObject
    COMPANY_NAME "YourCompany"
    PLUGIN_MANUFACTURER_CODE YrCo
    PLUGIN_CODE PDCu
    FORMATS AU VST3 Standalone AAX  # 添加 AAX
    PRODUCT_NAME "JYPad"
    DESCRIPTION "A 2D XY Pad controller with multiple draggable balls for PlugData"
    VERSION 0.1.0
    PLUGIN_MANUFACTURER "YourCompany"
    AAX_CATEGORY Effect  # 設置 AAX 類別
)
```

### 步驟 2：設置 AAX 類別

`AAX_CATEGORY` 可以是以下之一或多個：
- `None`
- `EQ`
- `Dynamics`
- `PitchShift`
- `Reverb`
- `Delay`
- `Modulation`
- `Harmonic`
- `NoiseReduction`
- `Dither`
- `SoundField`
- `HWGenerators`
- `SWGenerators`
- `WrappedPlugin`
- `Effect`
- `MIDIEffect`

對於 JYPad（MIDI 控制器），建議使用：
- `MIDIEffect`（如果主要用於 MIDI 控制）
- 或 `Effect`（如果作為音訊效果使用）

### 步驟 3：編譯

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

或者只編譯 AAX 版本：
```bash
cmake --build . --config Release --target PlugDataCustomObject_AAX
```

編譯完成後，AAX 插件會在：
```
build/PlugDataCustomObject_artefacts/AAX/JYPad.aaxplugin
```

### 步驟 4：安裝 AAX 插件

AAX 插件需要安裝到 Pro Tools 的插件目錄：

```bash
# macOS 系統目錄（需要管理員權限）
sudo cp -r build/PlugDataCustomObject_artefacts/AAX/JYPad.aaxplugin \
  /Library/Application\ Support/Avid/Audio/Plug-Ins/

# 或者用戶目錄
cp -r build/PlugDataCustomObject_artefacts/AAX/JYPad.aaxplugin \
  ~/Library/Application\ Support/Avid/Audio/Plug-Ins/
```

## 注意事項

1. **代碼簽名**：Pro Tools 可能需要簽名的 AAX 插件才能正常載入
2. **測試**：需要在 Pro Tools 中測試插件是否正常工作
3. **授權**：分發 AAX 插件可能需要 Avid 的授權（請查看 Avid 的授權條款）

## 驗證 AAX 插件

```bash
# 檢查插件結構
ls -la /Library/Application\ Support/Avid/Audio/Plug-Ins/JYPad.aaxplugin/

# 檢查可執行文件
file /Library/Application\ Support/Avid/Audio/Plug-Ins/JYPad.aaxplugin/Contents/MacOS/JYPad
```

## 故障排除

### 插件無法在 Pro Tools 中載入

1. 檢查插件是否在正確的目錄
2. 檢查插件是否已簽名
3. 查看 Pro Tools 的插件掃描日誌
4. 確保 Pro Tools 版本支援該 AAX 版本

### 編譯錯誤

如果遇到 AAX SDK 相關的編譯錯誤：
1. 確認 JUCE 版本包含 AAX SDK
2. 檢查 CMake 配置是否正確
3. 查看編譯錯誤日誌

## 參考資源

- [JUCE AAX Support](https://juce.com/discover/docs/audio-plugin-formats)
- [Avid AAX Documentation](https://developer.avid.com/)

