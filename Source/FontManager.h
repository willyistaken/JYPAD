#pragma once

#include <juce_graphics/juce_graphics.h>

//==============================================================================
/**
 * 字體管理器
 * 載入和管理中文字體
 */
class FontManager
{
public:
    static FontManager& getInstance();
    
    // 獲取中文字體
    juce::Font getChineseFont(float height = 14.0f);
    
    // 檢查字體是否已載入
    bool isFontLoaded() const { return chineseFont != nullptr; }
    
    // 獲取載入的字體名稱（用於調試）
    juce::String getLoadedFontName() const;
    
private:
    FontManager();
    ~FontManager() = default;
    
    juce::Typeface::Ptr chineseFont;
    
    // 嘗試從系統載入中文字體
    juce::Typeface::Ptr loadSystemChineseFont();
    
    // 嘗試從資源載入字體（如果字體文件嵌入在插件中）
    juce::Typeface::Ptr loadEmbeddedFont();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FontManager)
};

