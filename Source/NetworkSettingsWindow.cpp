#include "NetworkSettingsWindow.h"

//==============================================================================
NetworkSettingsWindow::NetworkSettingsWindow(PlugDataCustomObjectAudioProcessor& processor)
    : DocumentWindow("Network Settings",
                    juce::Colour(0xff1e1e1e),
                    DocumentWindow::allButtons,
                    true),
      audioProcessor(processor)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setSize(400, 250);
    setAlwaysOnTop(true);  // 設定為 always on top
    
    // 創建內容元件
    auto* content = new juce::Component();
    setContentOwned(content, true);
    content->setSize(400, 250);
    
    // OSC 設置區域
    oscGroup.setText("OSC Settings");
    oscGroup.setColour(juce::GroupComponent::outlineColourId, juce::Colour(0xff404040));
    oscGroup.setColour(juce::GroupComponent::textColourId, juce::Colours::white);
    content->addAndMakeVisible(&oscGroup);
    
    // IP 地址設置
    oscIpLabel.setText("IP Address:", juce::dontSendNotification);
    oscIpLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    oscIpLabel.setJustificationType(juce::Justification::centredLeft);
    content->addAndMakeVisible(&oscIpLabel);
    
    {
        juce::ScopedLock lock(audioProcessor.oscSettingsLock);
        oscIpEditor.setText(audioProcessor.oscSettings.ipAddress, juce::dontSendNotification);
    }
    oscIpEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    oscIpEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    oscIpEditor.onTextChange = [this] {
        {
            juce::ScopedLock lock(audioProcessor.oscSettingsLock);
            audioProcessor.oscSettings.ipAddress = oscIpEditor.getText();
        }
        audioProcessor.updateOSCConnection();
    };
    content->addAndMakeVisible(&oscIpEditor);
    
    // Port 設置
    oscPortLabel.setText("Port:", juce::dontSendNotification);
    oscPortLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    oscPortLabel.setJustificationType(juce::Justification::centredLeft);
    content->addAndMakeVisible(&oscPortLabel);
    
    {
        juce::ScopedLock lock(audioProcessor.oscSettingsLock);
        oscPortEditor.setText(juce::String(audioProcessor.oscSettings.port), juce::dontSendNotification);
    }
    oscPortEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    oscPortEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    oscPortEditor.onTextChange = [this] {
        int port = oscPortEditor.getText().getIntValue();
        if (port > 0 && port < 65536)
        {
            {
                juce::ScopedLock lock(audioProcessor.oscSettingsLock);
                audioProcessor.oscSettings.port = port;
            }
            audioProcessor.updateOSCConnection();
        }
    };
    content->addAndMakeVisible(&oscPortEditor);
    
    // 啟用/停用 OSC
    oscEnabledButton.setButtonText("Enable OSC");
    {
        juce::ScopedLock lock(audioProcessor.oscSettingsLock);
        oscEnabledButton.setToggleState(audioProcessor.oscSettings.enabled, juce::dontSendNotification);
    }
    oscEnabledButton.onClick = [this] {
        {
            juce::ScopedLock lock(audioProcessor.oscSettingsLock);
            audioProcessor.oscSettings.enabled = oscEnabledButton.getToggleState();
        }
        audioProcessor.updateOSCConnection();
    };
    content->addAndMakeVisible(&oscEnabledButton);
    
    // 測試按鈕
    oscTestButton.setButtonText("Test");
    oscTestButton.onClick = [this] {
        // 發送測試訊息
        audioProcessor.sendOSCMessage(999, 0.0f, 0.0f, 0.0f);
    };
    content->addAndMakeVisible(&oscTestButton);
    
    // 設定內容元件的佈局
    content->setBounds(0, 0, 400, 250);
    layoutContent(content);
}

NetworkSettingsWindow::~NetworkSettingsWindow()
{
}

//==============================================================================
void NetworkSettingsWindow::closeButtonPressed()
{
    setVisible(false);
}

//==============================================================================
void NetworkSettingsWindow::layoutContent(juce::Component* content)
{
    auto area = content->getLocalBounds().reduced(20);
    
    // OSC 設置區域
    auto oscArea = area.removeFromTop(180);
    oscGroup.setBounds(oscArea);
    
    auto oscContent = oscArea.reduced(15, 25);
    
    // IP 地址
    auto ipRow = oscContent.removeFromTop(25);
    oscIpLabel.setBounds(ipRow.removeFromLeft(80));
    oscIpEditor.setBounds(ipRow.removeFromLeft(150));
    
    oscContent.removeFromTop(5);
    
    // Port
    auto portRow = oscContent.removeFromTop(25);
    oscPortLabel.setBounds(portRow.removeFromLeft(80));
    oscPortEditor.setBounds(portRow.removeFromLeft(100));
    
    oscContent.removeFromTop(10);
    
    // 按鈕
    auto buttonRow = oscContent.removeFromTop(25);
    oscEnabledButton.setBounds(buttonRow.removeFromLeft(100));
    buttonRow.removeFromLeft(10);
    oscTestButton.setBounds(buttonRow.removeFromLeft(60));
}

