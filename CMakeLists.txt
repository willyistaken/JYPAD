cmake_minimum_required(VERSION 3.22)

project(PlugDataCustomObject VERSION 0.1.0)

# 設定 C++ 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加 JUCE
add_subdirectory(JUCE)

# 創建插件目標
juce_add_plugin(PlugDataCustomObject
    COMPANY_NAME "Jinyao Lin Audio Lab"
    BUNDLE_ID "com.jinyaolin.PlugDataCustomObject"  # Bundle ID 不能包含空格
    PLUGIN_MANUFACTURER_CODE YrCo
    PLUGIN_CODE PDCu
    FORMATS AU Standalone
    # AAX 和 VST3 暫時禁用，release 時再啟用以節省編譯時間
    # FORMATS AU VST3 Standalone AAX
    PRODUCT_NAME "JYPad"
    DESCRIPTION "A 2D XY Pad controller with multiple draggable balls for PlugData"
    VERSION 0.1.0
    PLUGIN_MANUFACTURER "Jinyao Lin Audio Lab"
    AAX_CATEGORY MIDIEffect  # AAX 插件類別：MIDI 效果
)

# 明確禁用 VST2 以避免衝突
set_target_properties(PlugDataCustomObject PROPERTIES
    JUCE_PLUGIN_BUILD_VST false
)

# 定義宏以忽略 VST3 參數 ID 警告（因為我們只構建 VST3，不構建 VST2）
target_compile_definitions(PlugDataCustomObject PRIVATE
    JUCE_IGNORE_VST3_MISMATCHED_PARAMETER_ID_WARNING=1
)

# 源文件
target_sources(PlugDataCustomObject PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/JYPad.cpp
    Source/JYPad.h
    Source/JYPadEditor.cpp
    Source/JYPadEditor.h
    Source/DataTable.cpp
    Source/DataTable.h
    Source/FontManager.cpp
    Source/FontManager.h
    Source/OSCDataWindow.cpp
    Source/OSCDataWindow.h
    Source/NetworkSettingsWindow.cpp
    Source/NetworkSettingsWindow.h
    Source/SourceEditWindow.cpp
    Source/SourceEditWindow.h
    Source/DebugLogger.h
)

# 包含目錄
target_include_directories(PlugDataCustomObject PUBLIC
    Source
)

# JUCE 模組
target_link_libraries(PlugDataCustomObject
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_osc
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)


