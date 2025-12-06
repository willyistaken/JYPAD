#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_osc/juce_osc.h>
#include "JYPad.h"
#include "DataTable.h"

//==============================================================================
/**
*/
class PlugDataCustomObjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PlugDataCustomObjectAudioProcessor();
    ~PlugDataCustomObjectAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    // JYPad 物件實例
    JYPad jyPad;
    
    //==============================================================================
    // 數據表格（會保存在插件狀態中）
    DataTable dataTable;
    
    //==============================================================================
    // Zoom scale（用於保存使用者的縮放設定）
    // 預設值 9.746 對應 displayRange = 1.0，顯示範圍為 -1 到 1
    float zoomScale = 9.746f;

    //==============================================================================
    // OSC 設置和發送器
    struct OSCSettings
    {
        juce::String ipAddress = "127.0.0.1";
        int port = 4002;
        bool enabled = true;
    };
    
    OSCSettings oscSettings;
    juce::OSCSender oscSender;
    
    // 更新 OSC 連接
    void updateOSCConnection();
    
    // 發送 OSC 訊息（當球移動時）
    // 使用球的 oscPrefix 作為地址前綴，後面接上 x, y, z 座標
    void sendOSCMessage(int ballId, float x, float y, float z = 0.0f);
    
    // OSC 設置的線程安全鎖（供 UI 使用）
    mutable juce::CriticalSection oscSettingsLock;

    //==============================================================================
    // 時間碼資訊（從 DAW 獲取）
    struct TimeCodeInfo
    {
        double bpm = 120.0;
        double timeInSeconds = 0.0;
        double ppqPosition = 0.0;  // PPQ (Pulses Per Quarter Note) 位置
        bool isPlaying = false;
        bool isValid = false;
        int timeSignatureNumerator = 4;
        int timeSignatureDenominator = 4;
        double ppqPositionOfLastBarStart = 0.0;  // 最後一個小節開始的 PPQ 位置
    };
    
    // 獲取時間碼資訊（線程安全）
    TimeCodeInfo getTimeCodeInfo() const;

private:
    //==============================================================================
    // 時間碼資訊緩存（在 processBlock 中更新，在 UI 中讀取）
    mutable juce::CriticalSection timeCodeLock;
    TimeCodeInfo cachedTimeCodeInfo;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlugDataCustomObjectAudioProcessor)
};
