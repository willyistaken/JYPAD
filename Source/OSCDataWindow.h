#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * OSC Data 視窗
 * 顯示數據表格的獨立視窗
 */
class OSCDataWindow : public juce::DocumentWindow,
                      public juce::TableListBoxModel,
                      public juce::ComponentListener
{
public:
    OSCDataWindow(PlugDataCustomObjectAudioProcessor& processor);
    ~OSCDataWindow() override;

    void closeButtonPressed() override;
    
    void componentMovedOrResized(juce::Component& component, bool wasMoved, bool wasResized) override;

    // TableListBoxModel 介面
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected, juce::Component* existingComponentToUpdate) override;

private:
    PlugDataCustomObjectAudioProcessor& audioProcessor;
    
    juce::TableListBox tableListBox;
    juce::TextButton addRowButton;
    juce::TextButton removeRowButton;
    
    void updateTableDisplay();
    void layoutContent(juce::Component* content);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OSCDataWindow)
};

