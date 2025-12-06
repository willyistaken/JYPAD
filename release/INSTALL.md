# JYPad v0.1.0 安裝指南

## 快速安裝

### macOS

1. **編譯專案**（如果還沒編譯）
```bash
cd /path/to/plugins/build
cmake --build . --config Release
```

2. **安裝插件**

**VST3 插件**：
```bash
cp -r build/PlugDataCustomObject_artefacts/VST3/JYPad.vst3 ~/Library/Audio/Plug-Ins/VST3/
```

**AU 插件**（可選）：
```bash
cp -r build/PlugDataCustomObject_artefacts/AU/JYPad.component ~/Library/Audio/Plug-Ins/Components/
```

3. **在 DAW 中使用**
   - 打開您的 DAW（Reaper, Logic Pro, Ableton Live 等）
   - 重新掃描插件
   - 在插件列表中尋找 "JYPad"
   - 載入並使用

## 驗證安裝

```bash
# 檢查 VST3
ls -la ~/Library/Audio/Plug-Ins/VST3/JYPad.vst3

# 檢查 AU
ls -la ~/Library/Audio/Plug-Ins/Components/JYPad.component
```

## 系統需求

- macOS 10.13 或更高版本
- 支援 VST3 或 AU 格式的 DAW

## 故障排除

如果插件無法載入：

1. 檢查權限：
```bash
chmod -R 755 ~/Library/Audio/Plug-Ins/VST3/JYPad.vst3
```

2. 在 DAW 中重新掃描插件

3. 重啟 DAW


