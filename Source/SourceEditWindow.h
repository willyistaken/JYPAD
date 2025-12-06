#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "JYPad.h"

//==============================================================================
/**
 * Add/Edit Source 視窗
 * 用於新增或編輯 Source 的資訊
 */
class SourceEditWindow : public juce::DialogWindow
{
public:
    struct SourceInfo
    {
        juce::String oscPrefix = "/track/1";  // 預設 prefix 為 /track/n
        juce::Colour color = juce::Colour(0xff4a90e2);
        juce::String sourceName = "";
        int sourceNumber = 1;
    };
    
    SourceEditWindow(const juce::String& title, const SourceInfo& initialInfo, 
                     std::function<void(const SourceInfo&)> onOk,
                     juce::Point<int> position = juce::Point<int>());
    ~SourceEditWindow() override;
    
    void closeButtonPressed() override;
    
    static void showModal(const juce::String& title, const SourceInfo& initialInfo,
                         std::function<void(const SourceInfo&)> onOk,
                         juce::Point<int> position = juce::Point<int>());
    
private:
    class ContentComponent : public juce::Component
    {
    public:
        ContentComponent(const SourceInfo& initialInfo, 
                        std::function<void(const SourceInfo&)> onOk,
                        std::function<void()> onCancel);
        ~ContentComponent() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
    private:
        juce::Label oscPrefixLabel;
        juce::TextEditor oscPrefixEditor;
        juce::Label colorLabel;
        juce::TextButton colorButton;
        juce::Label sourceNameLabel;
        juce::TextEditor sourceNameEditor;
        juce::Label sourceNumberLabel;
        juce::TextEditor sourceNumberEditor;
        juce::TextButton okButton;
        juce::TextButton cancelButton;
        
        SourceInfo currentInfo;
        std::function<void(const SourceInfo&)> onOkCallback;
        std::function<void()> onCancelCallback;
        
        void showColorPicker();
        void onOkClicked();
        void onCancelClicked();
    };
    
    std::unique_ptr<ContentComponent> content;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SourceEditWindow)
};

