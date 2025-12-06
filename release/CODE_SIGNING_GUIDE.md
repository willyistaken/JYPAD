# macOS 代碼簽名指南

## 概述

要對 macOS 應用程式進行代碼簽名，您需要：
1. Apple Developer 帳號（$99/年）
2. 代碼簽名證書
3. 可選：公證（Notarization）以完全繞過 Gatekeeper

## 步驟 1：註冊 Apple Developer 帳號

1. 前往：https://developer.apple.com/programs/
2. 點擊 "Enroll"
3. 使用您的 Apple ID 登入
4. 選擇個人或組織帳號
5. 支付年費 $99 USD
6. 等待審核通過（通常 24-48 小時）

## 步驟 2：創建證書

### 方法 A：使用 Xcode（推薦）

1. 打開 Xcode
2. 前往 `Xcode` → `Preferences` → `Accounts`
3. 點擊左下角的 `+` 添加您的 Apple ID
4. 選擇您的帳號，點擊 "Manage Certificates"
5. 點擊左下角的 `+` → 選擇 "Developer ID Application"
6. 證書會自動下載並安裝到 Keychain

### 方法 B：使用 Apple Developer 網站

1. 登入：https://developer.apple.com/account/
2. 前往 `Certificates, Identifiers & Profiles`
3. 點擊 `Certificates` → `+` 創建新證書
4. 選擇 "Developer ID Application"（用於分發）
5. 上傳 Certificate Signing Request (CSR)：
   ```bash
   # 在 Mac 上執行
   openssl req -new -newkey rsa:2048 -nodes -keyout private_key.pem -out certificate_request.csr
   ```
6. 下載證書並雙擊安裝到 Keychain

## 步驟 3：配置 CMakeLists.txt 進行自動簽名

在 `CMakeLists.txt` 中添加以下配置：

```cmake
# 代碼簽名配置（僅在 Release 模式下）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # 查找 codesign 工具
    find_program(CODESIGN_EXECUTABLE codesign)
    
    if(CODESIGN_EXECUTABLE)
        # 設置簽名身份（替換為您的 Developer ID）
        set(CODE_SIGN_IDENTITY "Developer ID Application: Your Name (TEAM_ID)")
        
        # 為 Standalone 應用程式添加簽名
        add_custom_command(TARGET PlugDataCustomObject_Standalone POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} 
                --force 
                --sign "${CODE_SIGN_IDENTITY}" 
                --deep
                --timestamp
                --options runtime
                "$<TARGET_BUNDLE_DIR:PlugDataCustomObject_Standalone>/PlugDataCustomObject_Standalone.app"
            COMMENT "Signing Standalone application"
        )
        
        # 為 VST3 插件添加簽名
        add_custom_command(TARGET PlugDataCustomObject_VST3 POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} 
                --force 
                --sign "${CODE_SIGN_IDENTITY}" 
                --deep
                --timestamp
                --options runtime
                "$<TARGET_BUNDLE_DIR:PlugDataCustomObject_VST3>/PlugDataCustomObject_VST3.vst3"
            COMMENT "Signing VST3 plugin"
        )
        
        # 為 AU 插件添加簽名
        add_custom_command(TARGET PlugDataCustomObject_AU POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} 
                --force 
                --sign "${CODE_SIGN_IDENTITY}" 
                --deep
                --timestamp
                --options runtime
                "$<TARGET_BUNDLE_DIR:PlugDataCustomObject_AU>/PlugDataCustomObject_AU.component"
            COMMENT "Signing AU plugin"
        )
    endif()
endif()
```

## 步驟 4：查找您的 Developer ID

執行以下命令查找您的證書：

```bash
# 列出所有 Developer ID 證書
security find-identity -v -p codesigning | grep "Developer ID Application"

# 輸出格式類似：
# 1) ABC123DEF456... "Developer ID Application: Your Name (TEAM_ID)"
```

將完整的身份字符串（包括引號內的內容）替換到 `CODE_SIGN_IDENTITY` 中。

## 步驟 5：手動簽名（如果自動簽名失敗）

### 簽名 Standalone 應用程式

```bash
codesign --force \
  --sign "Developer ID Application: Your Name (TEAM_ID)" \
  --deep \
  --timestamp \
  --options runtime \
  /path/to/JYPad.app
```

### 簽名 VST3 插件

```bash
codesign --force \
  --sign "Developer ID Application: Your Name (TEAM_ID)" \
  --deep \
  --timestamp \
  --options runtime \
  /path/to/JYPad.vst3
```

### 簽名 AU 插件

```bash
codesign --force \
  --sign "Developer ID Application: Your Name (TEAM_ID)" \
  --deep \
  --timestamp \
  --options runtime \
  /path/to/JYPad.component
```

## 步驟 6：驗證簽名

```bash
# 檢查簽名
codesign -dv --verbose=4 /path/to/JYPad.app

# 驗證簽名
codesign --verify --verbose /path/to/JYPad.app

# 檢查所有簽名
spctl --assess --verbose /path/to/JYPad.app
```

## 步驟 7：公證（Notarization）- 可選但推薦

公證可以讓應用程式在首次打開時不需要用戶手動允許。

### 創建 App-Specific Password

1. 前往：https://appleid.apple.com/
2. 登入您的 Apple ID
3. 前往 "App-Specific Passwords"
4. 創建一個新的密碼（用於 notarytool）

### 使用 notarytool 公證

```bash
# 創建 zip 文件（公證需要）
ditto -c -k --keepParent /path/to/JYPad.app JYPad.zip

# 提交公證
xcrun notarytool submit JYPad.zip \
  --apple-id "your@email.com" \
  --team-id "YOUR_TEAM_ID" \
  --password "app-specific-password" \
  --wait

# 檢查狀態
xcrun notarytool log <submission-id> \
  --apple-id "your@email.com" \
  --team-id "YOUR_TEAM_ID" \
  --password "app-specific-password"
```

### 釘書（Staple）公證票據

```bash
xcrun stapler staple /path/to/JYPad.app
xcrun stapler validate /path/to/JYPad.app
```

## 成本

- **Apple Developer Program**：$99 USD/年
- **公證**：免費（包含在 Developer Program 中）

## 免費替代方案

如果您不想支付 $99/年，可以使用：

1. **臨時證書**（免費，但有限制）：
   - 使用 Xcode 創建臨時證書
   - 僅在您的 Mac 上有效
   - 7 天後過期

2. **移除隔離屬性**（當前方法）：
   - 完全免費
   - 用戶需要手動執行 `xattr -dr com.apple.quarantine`
   - 適合個人使用或小範圍分發

## 自動化腳本

我們可以創建一個自動化腳本來處理簽名和公證。如果您有 Developer ID，我可以幫您創建這個腳本。

## 參考資源

- [Apple Developer Documentation](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [Code Signing Guide](https://developer.apple.com/library/archive/documentation/Security/Conceptual/CodeSigningGuide/)
- [Notarization Guide](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)

