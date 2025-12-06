#include "FontManager.h"

//==============================================================================
FontManager& FontManager::getInstance()
{
    static FontManager instance;
    return instance;
}

FontManager::FontManager()
{
    // 嘗試載入中文字體
    chineseFont = loadSystemChineseFont();
    
    // 如果系統字體載入失敗，嘗試載入嵌入的字體
    if (chineseFont == nullptr)
        chineseFont = loadEmbeddedFont();
}

//==============================================================================
juce::Typeface::Ptr FontManager::loadSystemChineseFont()
{
    // 在 macOS 上嘗試載入系統中文字體
   #if JUCE_MAC || JUCE_IOS
    // 首先嘗試從文件路徑直接載入（按優先順序）
    juce::StringArray fontPaths = {
        "/System/Library/Fonts/STHeiti Light.ttc",      // 黑體（最常見）
        "/System/Library/Fonts/STHeiti Medium.ttc",      // 黑體 Medium
        "/System/Library/Fonts/Supplemental/PingFang.ttc", // PingFang
        "/System/Library/Fonts/PingFang.ttc",
        "/System/Library/Fonts/STSong.ttc",              // 宋體
        "/Library/Fonts/PingFang.ttc"
    };
    
    // 也嘗試搜索 AssetsV2 目錄（macOS 動態字體位置）
    juce::File assetsDir("/System/Library/AssetsV2/com_apple_MobileAsset_Font7");
    if (assetsDir.exists())
    {
        juce::Array<juce::File> assetDirs;
        assetsDir.findChildFiles(assetDirs, juce::File::findDirectories, false);
        for (const auto& assetDir : assetDirs)
        {
            juce::File fontFile = assetDir.getChildFile("AssetData/PingFang.ttc");
            if (fontFile.existsAsFile())
                fontPaths.add(fontFile.getFullPathName());
            
            fontFile = assetDir.getChildFile("AssetData/STHEITI.ttf");
            if (fontFile.existsAsFile())
                fontPaths.add(fontFile.getFullPathName());
        }
    }
    
    for (const auto& fontPath : fontPaths)
    {
        juce::File fontFile(fontPath);
        if (fontFile.existsAsFile())
        {
            juce::MemoryBlock fontData;
            if (fontFile.loadFileAsData(fontData))
            {
                auto typeface = juce::Typeface::createSystemTypefaceFor(
                    fontData.getData(), fontData.getSize());
                if (typeface != nullptr)
                {
                    // 測試是否能顯示中文
                    juce::Font testFont(typeface);
                    juce::GlyphArrangement arr;
                    arr.addLineOfText(testFont, "測試", 0, 0);
                    if (arr.getNumGlyphs() > 0)
                    {
                        return typeface;
                    }
                }
            }
        }
    }
    
    // 如果文件路徑載入失敗，嘗試按名稱載入
    juce::StringArray fontNames = {
        "PingFang SC",
        "PingFang TC",
        "STHeiti",
        "STSong",
        "Hiragino Sans GB",
        "Arial Unicode MS",
        "Noto Sans CJK SC",
        "Source Han Sans SC"
    };
    
    for (const auto& fontName : fontNames)
    {
        auto font = juce::Font(fontName, 14.0f, juce::Font::plain);
        if (font.getTypefacePtr() != nullptr)
        {
            // 測試是否能顯示中文
            juce::GlyphArrangement arr;
            arr.addLineOfText(font, "測試", 0, 0);
            if (arr.getNumGlyphs() > 0)
            {
                return font.getTypefacePtr();
            }
        }
    }
   #elif JUCE_WINDOWS
    // Windows 字體路徑
    juce::StringArray fontPaths = {
        "C:/Windows/Fonts/msyh.ttc",      // 微軟雅黑
        "C:/Windows/Fonts/simhei.ttf",    // 黑體
        "C:/Windows/Fonts/simsun.ttc",    // 宋體
        "C:/Windows/Fonts/kaiti.ttf"      // 楷體
    };
    
    for (const auto& fontPath : fontPaths)
    {
        juce::File fontFile(fontPath);
        if (fontFile.existsAsFile())
        {
            juce::MemoryBlock fontData;
            if (fontFile.loadFileAsData(fontData))
            {
                auto typeface = juce::Typeface::createSystemTypefaceFor(
                    fontData.getData(), fontData.getSize());
                if (typeface != nullptr)
                {
                    juce::Font testFont(typeface);
                    juce::GlyphArrangement arr;
                    arr.addLineOfText(testFont, "測試", 0, 0);
                    if (arr.getNumGlyphs() > 0)
                    {
                        return typeface;
                    }
                }
            }
        }
    }
    
    // 按名稱載入
    juce::StringArray fontNames = {
        "Microsoft YaHei",
        "SimHei",
        "SimSun",
        "KaiTi",
        "Noto Sans CJK SC",
        "Source Han Sans SC"
    };
    
    for (const auto& fontName : fontNames)
    {
        auto font = juce::Font(fontName, 14.0f, juce::Font::plain);
        if (font.getTypefacePtr() != nullptr)
        {
            juce::GlyphArrangement arr;
            arr.addLineOfText(font, "測試", 0, 0);
            if (arr.getNumGlyphs() > 0)
            {
                return font.getTypefacePtr();
            }
        }
    }
   #elif JUCE_LINUX
    juce::StringArray fontNames = {
        "Noto Sans CJK SC",      // Noto Sans
        "Source Han Sans SC",     // 思源黑體
        "WenQuanYi Micro Hei",   // 文泉驛微米黑
        "WenQuanYi Zen Hei",      // 文泉驛正黑
        "AR PL UMing CN"          // AR PL 中楷
    };
    
    for (const auto& fontName : fontNames)
    {
        auto font = juce::Font(fontName, 14.0f, juce::Font::plain);
        if (font.getTypefacePtr() != nullptr)
        {
            // 測試是否能顯示中文
            juce::GlyphArrangement arr;
            arr.addLineOfText(font, "測試", 0, 0);
            if (arr.getNumGlyphs() > 0)
            {
                return font.getTypefacePtr();
            }
        }
    }
   #endif
    
    return nullptr;
}

//==============================================================================
juce::Typeface::Ptr FontManager::loadEmbeddedFont()
{
    // 這裡可以載入嵌入在插件中的字體文件
    // 例如使用 BinaryData 或資源文件
    // 目前返回 nullptr，如果需要可以添加字體文件到資源中
    
    // 範例：如果字體文件嵌入在 BinaryData 中
    // if (auto fontData = BinaryData::NotoSansCJK_ttf)
    // {
    //     return juce::Typeface::createSystemTypefaceFor(fontData, BinaryData::NotoSansCJK_ttfSize);
    // }
    
    return nullptr;
}

//==============================================================================
juce::Font FontManager::getChineseFont(float height)
{
    if (chineseFont != nullptr)
    {
        return juce::Font(chineseFont).withHeight(height);
    }
    
    // 如果沒有載入中文字體，返回默認字體
    return juce::Font(juce::FontOptions().withHeight(height));
}

//==============================================================================
juce::String FontManager::getLoadedFontName() const
{
    if (chineseFont != nullptr)
        return chineseFont->getName();
    return "Default (No Chinese Font)";
}

