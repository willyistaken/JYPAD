#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "JYPadEditor.h"
#include "OSCDataWindow.h"
#include "NetworkSettingsWindow.h"

//==============================================================================
/**
*/
class PlugDataCustomObjectAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                                   public juce::Timer
{
public:
    PlugDataCustomObjectAudioProcessorEditor (PlugDataCustomObjectAudioProcessor&);
    ~PlugDataCustomObjectAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    // Timer 回調，用於更新時間碼
    void timerCallback() override;
    
    // 當視窗可見性改變時，同步關閉子視窗
    void visibilityChanged() override;
    
private:
    // 追蹤之前的播放狀態，用於檢測狀態變化
    bool wasPlaying = false;
    double lastMidiTime = -1.0;  // 追蹤上一次的 MIDI time

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PlugDataCustomObjectAudioProcessor& audioProcessor;

    // JYPad 編輯器
    JYPadEditor jyPadEditor;
    
    // 時間碼顯示（左上角）
    juce::Label timeCodeLabel;      // 第一行：Time Code (MM:SS:mmm)
    juce::Label midiTimeLabel;       // 第二行：MIDI Time (Bar:Beats:Ticks)
    juce::Label bpmInfoLabel;        // 右側：BPM 等資訊
    
    // 輸出顯示區域
    juce::Label outputLabel;
    juce::TextEditor outputText;
    
    // OSC Data 視窗按鈕
    juce::TextButton openOSCDataButton;
    std::unique_ptr<OSCDataWindow> oscDataWindow;
    
    // Network Settings 視窗按鈕
    juce::TextButton openNetworkSettingsButton;
    std::unique_ptr<NetworkSettingsWindow> networkSettingsWindow;
    
    // 自定義按鈕樣式
    struct ButtonLookAndFeel : public juce::LookAndFeel_V4
    {
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, 
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override
        {
            auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
            auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
                                                .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);
            
            if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
                baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);
            
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            
            g.setColour(baseColour.contrasting(0.2f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        }
        
        void drawButtonText(juce::Graphics& g, juce::TextButton& button, 
                           bool shouldDrawButtonAsHighlighted, [[maybe_unused]] bool shouldDrawButtonAsDown) override
        {
            juce::Font font(12.0f, juce::Font::bold);
            g.setFont(font);
            g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                                   : juce::TextButton::textColourOffId)
                           .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));
            
            const int yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
            const int cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;
            const int fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
            const int leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
            const int rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
            const int textWidth = button.getWidth() - leftIndent - rightIndent;
            
            if (textWidth > 0)
                g.drawFittedText(button.getButtonText(),
                                leftIndent, yIndent, textWidth, button.getHeight() - yIndent * 2,
                                juce::Justification::centred, 2);
        }
    };
    
    ButtonLookAndFeel buttonLookAndFeel;
    
    // 格式化時間碼字串
    juce::String formatTimeCode(double timeInSeconds) const;
    juce::String formatMIDITime(double ppqPosition, double bpm) const;
    juce::String formatBPMInfo(double bpm, bool isPlaying) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlugDataCustomObjectAudioProcessorEditor)
};
