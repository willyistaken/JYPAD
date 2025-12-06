#!/bin/bash

# JYPad macOS 權限修復腳本
# 用法: ./fix_permissions.sh /path/to/JYPad.app

if [ $# -eq 0 ]; then
    echo "用法: $0 /path/to/JYPad.app"
    echo "例如: $0 ~/Downloads/JYPad.app"
    exit 1
fi

APP_PATH="$1"

if [ ! -d "$APP_PATH" ]; then
    echo "錯誤: 找不到應用程式: $APP_PATH"
    exit 1
fi

echo "正在修復 JYPad 應用程式權限..."
echo "應用程式路徑: $APP_PATH"

# 移除隔離屬性
echo "1. 移除隔離屬性..."
xattr -dr com.apple.quarantine "$APP_PATH" 2>/dev/null
if [ $? -eq 0 ]; then
    echo "   ✓ 隔離屬性已移除"
else
    echo "   ⚠ 無法移除隔離屬性（可能已經移除或沒有權限）"
fi

# 設置執行權限
echo "2. 設置執行權限..."
chmod -R +x "$APP_PATH/Contents/MacOS" 2>/dev/null
if [ $? -eq 0 ]; then
    echo "   ✓ 執行權限已設置"
else
    echo "   ⚠ 無法設置執行權限"
fi

# 檢查應用程式
echo "3. 檢查應用程式..."
if [ -f "$APP_PATH/Contents/MacOS/JYPad" ]; then
    echo "   ✓ 應用程式可執行文件存在"
    file "$APP_PATH/Contents/MacOS/JYPad"
else
    echo "   ✗ 錯誤: 找不到可執行文件"
    exit 1
fi

echo ""
echo "完成！現在可以嘗試打開應用程式了。"
echo ""
echo "如果仍然無法打開，請嘗試："
echo "1. 按住 Control 鍵並點擊應用程式，選擇「打開」"
echo "2. 前往「系統偏好設定」→「安全性與隱私」→「仍要打開」"
echo ""


