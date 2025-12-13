cmake_minimum_required(VERSION 3.22)

project(PlugDataCustomObject VERSION 0.1.0)

# -----------------------------------------------------------------------------
# macOS 環境與編譯器限制設定 (針對 User 環境: Apple Clang 17.0.0 + CLI Tools)
# -----------------------------------------------------------------------------
if(APPLE)
    # 0. 禁止 Xcode Generator (避免開啟 Xcode)
    if(CMAKE_GENERATOR MATCHES "Xcode")
        message(FATAL_ERROR "此專案設定為避免開啟 Xcode。請使用 'Unix Makefiles' 或 'Ninja' Generator。\n請執行: cmake -G \"Unix Makefiles\" ..")
    endif()

    # 1. 架構限制：僅構建 arm64 (避免 Universal Build 尋找 x86 庫)
    set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for macOS" FORCE)

    # 2. SDK 限制：強制使用 Command Line Tools SDK，避免觸發 Xcode App 啟動
    set(CLI_SDK_PATH "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk")
    if(EXISTS "${CLI_SDK_PATH}")
        set(CMAKE_OSX_SYSROOT "${CLI_SDK_PATH}" CACHE PATH "Sysroot used for OSX support" FORCE)
        message(STATUS "Using Command Line Tools SDK: ${CMAKE_OSX_SYSROOT}")
    endif()

    # 3. 編譯器路徑提示 (優先使用 /usr/bin/clang++)
    if(NOT DEFINED CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "/usr/bin/clang++")
    endif()
endif()


# 設定 C++ 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "17.0.0")
        message(STATUS "Compiler Check: AppleClang version ${CMAKE_CXX_COMPILER_VERSION} (Requirement: >= 17.0.0)")
    else()
        message(STATUS "Compiler Check: Valid AppleClang version ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
endif()

# 添加 JUCE
add_subdirectory(JUCE)

# 創建插件目標
juce_add_plugin(PlugDataCustomObject
    COMPANY_NAME "Jinyao Lin Audio Lab"
    BUNDLE_ID "com.jinyaolin.PlugDataCustomObject"  # Bundle ID 不能包含空格
    PLUGIN_MANUFACTURER_CODE YrCo
    PLUGIN_CODE PDCu
    FORMATS AU VST3 Standalone
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


