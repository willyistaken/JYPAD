#include "OSCDataWindow.h"

//==============================================================================
OSCDataWindow::OSCDataWindow(PlugDataCustomObjectAudioProcessor& processor)
    : DocumentWindow("OSC DATA",
                    juce::Colour(0xff1e1e1e),
                    DocumentWindow::allButtons,
                    true),
      audioProcessor(processor)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setSize(600, 400);
    setAlwaysOnTop(true);  // 設定為 always on top
    
    // 創建內容元件
    auto* content = new juce::Component();
    setContentOwned(content, true);
    content->setSize(600, 400);
    
    // 設定表格列表
    tableListBox.setModel(this);
    tableListBox.getHeader().addColumn("Name", 1, 100, 50, 200);
    tableListBox.getHeader().addColumn("Value 1", 2, 80, 50, 150);
    tableListBox.getHeader().addColumn("Value 2", 3, 80, 50, 150);
    tableListBox.getHeader().addColumn("Note", 4, 150, 50, 300);
    content->addAndMakeVisible(&tableListBox);
    
    // 設定按鈕
    addRowButton.setButtonText("Add Row");
    addRowButton.onClick = [this] {
        audioProcessor.dataTable.addRow("New Row", 0.0f, 0.0f, "");
        updateTableDisplay();
    };
    content->addAndMakeVisible(&addRowButton);
    
    removeRowButton.setButtonText("Remove");
    removeRowButton.onClick = [this] {
        int selectedRow = tableListBox.getSelectedRow();
        if (selectedRow >= 0)
        {
            audioProcessor.dataTable.removeRow(selectedRow);
            updateTableDisplay();
        }
    };
    content->addAndMakeVisible(&removeRowButton);
    
    // 設定內容元件的佈局
    content->setBounds(0, 0, 600, 400);
    content->addComponentListener(this);
    
    // 初始佈局
    layoutContent(content);
    
    updateTableDisplay();
}

void OSCDataWindow::componentMovedOrResized(juce::Component& component, [[maybe_unused]] bool wasMoved, bool wasResized)
{
    if (wasResized && component.getParentComponent() == getContentComponent())
    {
        layoutContent(&component);
    }
}

void OSCDataWindow::layoutContent(juce::Component* content)
{
    auto area = content->getLocalBounds().reduced(10);
    
    // 按鈕區域
    auto buttonArea = area.removeFromTop(30);
    addRowButton.setBounds(buttonArea.removeFromLeft(80).reduced(2));
    removeRowButton.setBounds(buttonArea.removeFromLeft(80).reduced(2));
    
    area.removeFromTop(5);
    
    // 表格列表
    tableListBox.setBounds(area);
}

OSCDataWindow::~OSCDataWindow()
{
}

//==============================================================================
void OSCDataWindow::closeButtonPressed()
{
    setVisible(false);
}

//==============================================================================
void OSCDataWindow::updateTableDisplay()
{
    tableListBox.updateContent();
    tableListBox.repaint();
}

//==============================================================================
int OSCDataWindow::getNumRows()
{
    return audioProcessor.dataTable.getNumRows();
}

void OSCDataWindow::paintRowBackground(juce::Graphics& g, int rowNumber, [[maybe_unused]] int width, [[maybe_unused]] int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff404040));
    else if (rowNumber % 2 == 0)
        g.fillAll(juce::Colour(0xff2a2a2a));
    else
        g.fillAll(juce::Colour(0xff252525));
}

void OSCDataWindow::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, [[maybe_unused]] bool rowIsSelected)
{
    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    
    if (rowNumber < audioProcessor.dataTable.getNumRows())
    {
        const auto& row = audioProcessor.dataTable.getRow(rowNumber);
        
        juce::String text;
        switch (columnId)
        {
            case 1: text = row.name; break;
            case 2: text = juce::String(row.value1, 3); break;
            case 3: text = juce::String(row.value2, 3); break;
            case 4: text = row.note; break;
        }
        
        g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}

juce::Component* OSCDataWindow::refreshComponentForCell([[maybe_unused]] int rowNumber, [[maybe_unused]] int columnId, [[maybe_unused]] bool isRowSelected, [[maybe_unused]] juce::Component* existingComponentToUpdate)
{
    return nullptr;
}

