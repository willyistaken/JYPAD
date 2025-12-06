#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Network Settings 視窗
 * 顯示和編輯 OSC 網路設置的獨立視窗
 */
class NetworkSettingsWindow : public juce::DocumentWindow
{
public:
    NetworkSettingsWindow(PlugDataCustomObjectAudioProcessor& processor);
    ~NetworkSettingsWindow() override;

    void closeButtonPressed() override;

private:
    PlugDataCustomObjectAudioProcessor& audioProcessor;
    
    juce::GroupComponent oscGroup;
    juce::Label oscIpLabel;
    juce::TextEditor oscIpEditor;
    juce::Label oscPortLabel;
    juce::TextEditor oscPortEditor;
    juce::ToggleButton oscEnabledButton;
    juce::TextButton oscTestButton;
    
    void layoutContent(juce::Component* content);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetworkSettingsWindow)
};

