# JYPad v0.1.0 發布說明

## 版本資訊
- **版本號**：v0.1.0
- **發布日期**：2024
- **構建類型**：Release

## 主要功能

### 核心功能
- ✅ 2D XY 平面控制器，支援多個可拖動球體
- ✅ MIDI 錄製與回放功能
- ✅ OSC 通信支援（可自定義 OSC 前綴）
- ✅ DAW 時間碼同步
- ✅ 數據持久化（自動保存/載入）

### 球體管理
- ✅ 添加、編輯、刪除球體
- ✅ 自定義球體顏色、名稱、OSC 前綴
- ✅ Mute、Solo、Recording 狀態控制

### MIDI 功能
- ✅ 錄製球體移動軌跡
- ✅ 根據 MIDI 時間自動回放
- ✅ 插值功能（Tween to next）
- ✅ 事件管理（Clear Events, Set Position）

### 用戶界面
- ✅ 時間碼顯示（MIDI Time + Time Code）
- ✅ BPM 資訊顯示
- ✅ OSC Data 數據表格視窗
- ✅ Network Settings 網路設定視窗

## 技術改進

- ✅ Release 版本已禁用 Debug Log（提升性能）
- ✅ 優化的插值算法（根據錄製頻率自動調整）
- ✅ 改進的狀態管理（向後兼容舊版本數據）

## 已知問題

- 無

## 系統需求

- macOS 10.13 或更高版本
- 支援 VST3 或 AU 格式的 DAW

## 安裝

請參考 `INSTALL.md` 文件獲取詳細安裝說明。

## 使用說明

請參考 `README.md` 文件獲取完整使用說明。

---

**JYPad v0.1.0** - 強大的 2D 控制器插件

