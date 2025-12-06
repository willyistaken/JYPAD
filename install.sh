#!/bin/bash

# XYPad 安裝腳本
# 自動將 XYPad 插件安裝到系統插件目錄

set -e

echo "=========================================="
echo "XYPad 插件安裝腳本"
echo "=========================================="

# 檢查構建目錄
BUILD_DIR="build/PlugDataCustomObject_artefacts"
if [ ! -d "$BUILD_DIR" ]; then
    echo "錯誤: 找不到構建目錄 $BUILD_DIR"
    echo "請先編譯專案: cd build && cmake --build . --config Release"
    exit 1
fi

# 創建插件目錄（如果不存在）
VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"

mkdir -p "$VST3_DIR"
mkdir -p "$AU_DIR"

# 安裝 VST3
if [ -d "$BUILD_DIR/VST3/XYPad.vst3" ]; then
    echo "正在安裝 VST3 插件..."
    if [ -d "$VST3_DIR/XYPad.vst3" ]; then
        echo "  移除舊版本..."
        rm -rf "$VST3_DIR/XYPad.vst3"
    fi
    cp -r "$BUILD_DIR/VST3/XYPad.vst3" "$VST3_DIR/"
    echo "  ✓ VST3 插件已安裝到: $VST3_DIR/XYPad.vst3"
else
    echo "  警告: 找不到 VST3 插件"
fi

# 安裝 AU
if [ -d "$BUILD_DIR/AU/XYPad.component" ]; then
    echo "正在安裝 AU 插件..."
    if [ -d "$AU_DIR/XYPad.component" ]; then
        echo "  移除舊版本..."
        rm -rf "$AU_DIR/XYPad.component"
    fi
    cp -r "$BUILD_DIR/AU/XYPad.component" "$AU_DIR/"
    echo "  ✓ AU 插件已安裝到: $AU_DIR/XYPad.component"
else
    echo "  警告: 找不到 AU 插件"
fi

# 設置權限
if [ -d "$VST3_DIR/XYPad.vst3" ]; then
    chmod -R 755 "$VST3_DIR/XYPad.vst3"
fi
if [ -d "$AU_DIR/XYPad.component" ]; then
    chmod -R 755 "$AU_DIR/XYPad.component"
fi

echo ""
echo "=========================================="
echo "安裝完成！"
echo "=========================================="
echo ""
echo "VST3 位置: $VST3_DIR/XYPad.vst3"
echo "AU 位置:   $AU_DIR/XYPad.component"
echo ""
echo "請在您的 DAW 或 PlugData 中重新掃描插件。"
echo ""

