#include "SourceEditWindow.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
SourceEditWindow::SourceEditWindow(const juce::String& title, const SourceInfo& initialInfo,
                                   std::function<void(const SourceInfo&)> onOk,
                                   juce::Point<int> position)
    : DialogWindow(title, juce::Colour(0xff1e1e1e), true, true)
{
    content = std::make_unique<ContentComponent>(initialInfo, onOk, [this]() {
        closeButtonPressed();
    });
    setContentOwned(content.get(), true);
    setResizable(false, false);
    setSize(400, 300);
    setAlwaysOnTop(true);  // 設定為 always on top
    
    if (position.x != 0 || position.y != 0)
    {
        // 在指定位置顯示（滑鼠位置），確保視窗不會超出螢幕
        juce::Rectangle<int> screenArea = juce::Desktop::getInstance().getDisplays().getMainDisplay().userArea;
        // 視窗大小是 400x300，所以從滑鼠位置減去一半寬度和高度
        int x = position.x - 200;
        int y = position.y - 150;
        
        // 確保視窗不會超出螢幕邊界
        x = juce::jlimit(screenArea.getX(), screenArea.getRight() - 400, x);
        y = juce::jlimit(screenArea.getY(), screenArea.getBottom() - 300, y);
        
        setTopLeftPosition(x, y);
    }
    else
    {
        // 居中顯示
        centreWithSize(400, 300);
    }
}

void SourceEditWindow::showModal(const juce::String& title, const SourceInfo& initialInfo,
                                 std::function<void(const SourceInfo&)> onOk,
                                 juce::Point<int> position)
{
    auto* window = new SourceEditWindow(title, initialInfo, onOk, position);
    window->enterModalState(true, nullptr, true);
    window->setVisible(true);
    // 視窗會在 closeButtonPressed 或 cancel 時自動清理
}

SourceEditWindow::~SourceEditWindow()
{
}

void SourceEditWindow::closeButtonPressed()
{
    setVisible(false);
    exitModalState(0);
}

//==============================================================================
SourceEditWindow::ContentComponent::ContentComponent(const SourceInfo& initialInfo,
                                                    std::function<void(const SourceInfo&)> onOk,
                                                    std::function<void()> onCancel)
    : currentInfo(initialInfo), onOkCallback(onOk), onCancelCallback(onCancel)
{
    // OSC Prefix
    oscPrefixLabel.setText("OSC Prefix:", juce::dontSendNotification);
    oscPrefixLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    oscPrefixLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&oscPrefixLabel);
    
    oscPrefixEditor.setText(initialInfo.oscPrefix, juce::dontSendNotification);
    oscPrefixEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    oscPrefixEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    addAndMakeVisible(&oscPrefixEditor);
    
    // Color
    colorLabel.setText("Color:", juce::dontSendNotification);
    colorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    colorLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&colorLabel);
    
    colorButton.setButtonText("");
    colorButton.setColour(juce::TextButton::buttonColourId, initialInfo.color);
    colorButton.onClick = [this]() { showColorPicker(); };
    addAndMakeVisible(&colorButton);
    
    // Source Name
    sourceNameLabel.setText("Source Name:", juce::dontSendNotification);
    sourceNameLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    sourceNameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&sourceNameLabel);
    
    sourceNameEditor.setText(initialInfo.sourceName, juce::dontSendNotification);
    sourceNameEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    sourceNameEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    addAndMakeVisible(&sourceNameEditor);
    
    // Source Number
    sourceNumberLabel.setText("Source Number:", juce::dontSendNotification);
    sourceNumberLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    sourceNumberLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(&sourceNumberLabel);
    
    sourceNumberEditor.setText(juce::String(initialInfo.sourceNumber), juce::dontSendNotification);
    sourceNumberEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff2a2a2a));
    sourceNumberEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    addAndMakeVisible(&sourceNumberEditor);
    
    // Buttons
    okButton.setButtonText("OK");
    okButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a90e2));
    okButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    okButton.onClick = [this]() { onOkClicked(); };
    addAndMakeVisible(&okButton);
    
    cancelButton.setButtonText("Cancel");
    cancelButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    cancelButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    cancelButton.onClick = [this]() { onCancelClicked(); };
    addAndMakeVisible(&cancelButton);
}

SourceEditWindow::ContentComponent::~ContentComponent()
{
}

void SourceEditWindow::ContentComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void SourceEditWindow::ContentComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // OSC Prefix
    auto prefixRow = area.removeFromTop(30);
    oscPrefixLabel.setBounds(prefixRow.removeFromLeft(120));
    oscPrefixEditor.setBounds(prefixRow);
    
    area.removeFromTop(10);
    
    // Color
    auto colorRow = area.removeFromTop(30);
    colorLabel.setBounds(colorRow.removeFromLeft(120));
    colorButton.setBounds(colorRow.removeFromLeft(60));
    
    area.removeFromTop(10);
    
    // Source Name
    auto nameRow = area.removeFromTop(30);
    sourceNameLabel.setBounds(nameRow.removeFromLeft(120));
    sourceNameEditor.setBounds(nameRow);
    
    area.removeFromTop(10);
    
    // Source Number
    auto numberRow = area.removeFromTop(30);
    sourceNumberLabel.setBounds(numberRow.removeFromLeft(120));
    sourceNumberEditor.setBounds(numberRow.removeFromLeft(100));
    
    area.removeFromTop(20);
    
    // Buttons
    auto buttonRow = area.removeFromTop(30);
    cancelButton.setBounds(buttonRow.removeFromRight(80));
    buttonRow.removeFromRight(10);
    okButton.setBounds(buttonRow.removeFromRight(80));
}

void SourceEditWindow::ContentComponent::showColorPicker()
{
    class ColourSelectorListener : public juce::ChangeListener
    {
    public:
        ColourSelectorListener(SourceEditWindow::ContentComponent* comp)
            : component(comp) {}
        
        void changeListenerCallback(juce::ChangeBroadcaster* source) override
        {
            if (auto* cs = dynamic_cast<juce::ColourSelector*>(source))
            {
                component->currentInfo.color = cs->getCurrentColour();
                component->colorButton.setColour(juce::TextButton::buttonColourId, component->currentInfo.color);
            }
        }
        
    private:
        SourceEditWindow::ContentComponent* component;
    };
    
    auto colourSelector = std::make_unique<juce::ColourSelector>(
        juce::ColourSelector::showColourAtTop | 
        juce::ColourSelector::showSliders | 
        juce::ColourSelector::showColourspace);
    
    colourSelector->setCurrentColour(currentInfo.color);
    colourSelector->setSize(300, 400);
    
    auto* listener = new ColourSelectorListener(this);
    colourSelector->addChangeListener(listener);
    
    juce::CallOutBox::launchAsynchronously(std::move(colourSelector), 
                                           colorButton.getScreenBounds(), nullptr);
}

void SourceEditWindow::ContentComponent::onOkClicked()
{
    currentInfo.oscPrefix = oscPrefixEditor.getText();
    currentInfo.sourceName = sourceNameEditor.getText();
    currentInfo.sourceNumber = sourceNumberEditor.getText().getIntValue();
    
    if (onOkCallback)
        onOkCallback(currentInfo);
    
    // 關閉視窗
    if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
    {
        dw->exitModalState(0);
        dw->setVisible(false);
    }
}

void SourceEditWindow::ContentComponent::onCancelClicked()
{
    if (onCancelCallback)
        onCancelCallback();
    // 關閉視窗
    if (auto* dw = findParentComponentOfClass<juce::DialogWindow>())
    {
        dw->exitModalState(0);
        dw->setVisible(false);
    }
}

