# macOS 權限問題解決方案

## 問題說明

在 macOS 上，未簽名的應用程式在另一台 Mac 上運行時，可能會被 Gatekeeper 阻止執行。這是 macOS 的安全機制。

## 解決方案

### 方法 1：移除隔離屬性（推薦）

在**接收應用程式的 Mac** 上執行以下命令：

```bash
# 如果應用程式在 Downloads 或其他位置
xattr -dr com.apple.quarantine /path/to/JYPad.app

# 例如，如果應用程式在 Downloads 資料夾：
xattr -dr com.apple.quarantine ~/Downloads/JYPad.app
```

### 方法 2：通過系統偏好設定允許

1. 嘗試打開應用程式
2. 如果出現「無法打開，因為來自身份不明的開發者」的提示
3. 前往「系統偏好設定」→「安全性與隱私」
4. 點擊「仍要打開」按鈕

### 方法 3：使用右鍵選單

1. 按住 `Control` 鍵（或右鍵點擊）應用程式
2. 選擇「打開」
3. 在彈出的對話框中點擊「打開」

### 方法 4：臨時禁用 Gatekeeper（不推薦，僅用於測試）

```bash
# 僅用於測試，不建議在生產環境使用
sudo spctl --master-disable
```

## 自動化腳本

我們提供了一個自動化腳本來處理這個問題。請在接收應用程式的 Mac 上執行：

```bash
# 下載並執行 fix_permissions.sh
chmod +x fix_permissions.sh
./fix_permissions.sh /path/to/JYPad.app
```

## 長期解決方案

要完全解決這個問題，需要對應用程式進行代碼簽名。這需要：

1. Apple Developer 帳號（$99/年）
2. 使用 `codesign` 工具簽名應用程式
3. 可選：進行公證（notarization）

對於開源項目或個人使用，方法 1 是最簡單的解決方案。

