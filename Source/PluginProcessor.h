#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_osc/juce_osc.h>
#include "JYPad.h"
#include "DataTable.h"

//==============================================================================
/**
*/
// 前向聲明
class PlugDataCustomObjectAudioProcessorEditor;

class PlugDataCustomObjectAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PlugDataCustomObjectAudioProcessor();
    ~PlugDataCustomObjectAudioProcessor() override;
    
    // 設置 Editor 指針（用於記錄 OSC 訊息）
    void setEditor(PlugDataCustomObjectAudioProcessorEditor* editor) { oscMessageEditor = editor; }

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
    // 格式：{osc_prefix}/xy x y（暫時不發送 z 值）
    // TODO: 未來實作 z 軸錄影後，可以改為發送 x, y, z
    void sendOSCMessage(int ballId, float x, float y, float z = 0.0f);
    
    // 發送 mute/solo OSC 訊息
    // 格式：{osc_prefix}/n/mute 1 或 0
    // 格式：{osc_prefix}/n/solo 1 或 0
    // 其中 n 是 source number
    void sendMuteSoloOSCMessage(int ballId, bool isMute, bool isSolo);
    
    // OSC 設置的線程安全鎖（供 UI 使用）
    mutable juce::CriticalSection oscSettingsLock;

    //==============================================================================
    // 時間碼資訊（從 DAW 獲取）
    struct TimeCodeInfo
    {
        std::atomic<double> bpm { 120.0 };
        std::atomic<double> timeInSeconds { 0.0 };
        std::atomic<double> ppqPosition { 0.0 };  // PPQ (Pulses Per Quarter Note) 位置
        std::atomic<bool> isPlaying { false };
        std::atomic<bool> isValid { false };
        std::atomic<int> timeSignatureNumerator { 4 };
        std::atomic<int> timeSignatureDenominator { 4 };
        std::atomic<double> ppqPositionOfLastBarStart { 0.0 };  // 最後一個小節開始的 PPQ 位置
        
        // Copy constructor needed for atomic members
        TimeCodeInfo() = default;
        TimeCodeInfo(const TimeCodeInfo& other) {
            bpm.store(other.bpm.load());
            timeInSeconds.store(other.timeInSeconds.load());
            ppqPosition.store(other.ppqPosition.load());
            isPlaying.store(other.isPlaying.load());
            isValid.store(other.isValid.load());
            timeSignatureNumerator.store(other.timeSignatureNumerator.load());
            timeSignatureDenominator.store(other.timeSignatureDenominator.load());
            ppqPositionOfLastBarStart.store(other.ppqPositionOfLastBarStart.load());
        }
        
        // Assignment operator needed for atomic members
        TimeCodeInfo& operator=(const TimeCodeInfo& other) {
            if (this != &other) {
                bpm.store(other.bpm.load());
                timeInSeconds.store(other.timeInSeconds.load());
                ppqPosition.store(other.ppqPosition.load());
                isPlaying.store(other.isPlaying.load());
                isValid.store(other.isValid.load());
                timeSignatureNumerator.store(other.timeSignatureNumerator.load());
                timeSignatureDenominator.store(other.timeSignatureDenominator.load());
                ppqPositionOfLastBarStart.store(other.ppqPositionOfLastBarStart.load());
            }
            return *this;
        }
    };
    
    // 獲取時間碼資訊（線程安全）
    TimeCodeInfo getTimeCodeInfo() const;

private:
    //==============================================================================
    // 時間碼資訊緩存（在 processBlock 中更新，在 UI 中讀取）
    TimeCodeInfo cachedTimeCodeInfo;
    
    // Editor 指針（用於記錄 OSC 訊息）
    PlugDataCustomObjectAudioProcessorEditor* oscMessageEditor = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlugDataCustomObjectAudioProcessor)
};
