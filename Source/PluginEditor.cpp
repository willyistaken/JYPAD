#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FontManager.h"
#include "DebugLogger.h"

//==============================================================================
PlugDataCustomObjectAudioProcessorEditor::PlugDataCustomObjectAudioProcessorEditor (PlugDataCustomObjectAudioProcessor& p)
    : AudioProcessorEditor (&p), 
      audioProcessor (p),
      jyPadEditor (p.jyPad, p)
{
    DEBUG_LOG("PluginEditor: Constructor started - STEP 1");
    
    try
    {
        DEBUG_LOG("PluginEditor: JYPadEditor initialized - STEP 2");
        
        // 設定視窗大小（移除 OSC 設置後可以縮小）
        DEBUG_LOG("PluginEditor: Setting window size - STEP 3");
        setSize (600, 650);
        DEBUG_LOG("PluginEditor: Window size set - STEP 4");

    // 設定 MIDI 時間標籤（第一行：Bar.Beat.Ticks）
    DEBUG_LOG("PluginEditor: Setting up midiTimeLabel - STEP 5");
    midiTimeLabel.setText("1.1.000", juce::dontSendNotification);
    midiTimeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ff00));  // 綠色 digital 風格
    midiTimeLabel.setJustificationType(juce::Justification::centredRight);
    // 使用 monospace font 作為 digital font
    juce::Font digitalFont = juce::Font(juce::FontOptions().withHeight(18.0f));
    digitalFont = digitalFont.boldened();
    digitalFont.setTypefaceName("Courier New");  // 嘗試使用 monospace font
    midiTimeLabel.setFont(digitalFont);
    addAndMakeVisible(&midiTimeLabel);
    DEBUG_LOG("PluginEditor: midiTimeLabel added - STEP 6");
    
    // 設定時間碼標籤（第二行：m:ss.SSS）
    DEBUG_LOG("PluginEditor: Setting up timeCodeLabel - STEP 6.5");
    timeCodeLabel.setText("0:00.000", juce::dontSendNotification);
    timeCodeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ff00));  // 綠色 digital 風格
    timeCodeLabel.setJustificationType(juce::Justification::centredRight);
    timeCodeLabel.setFont(digitalFont);
    addAndMakeVisible(&timeCodeLabel);
    DEBUG_LOG("PluginEditor: timeCodeLabel added - STEP 6.6");
    
    // 設定 BPM 資訊標籤（右側）
    DEBUG_LOG("PluginEditor: Setting up bpmInfoLabel - STEP 6.7");
    bpmInfoLabel.setText("120.0 BPM", juce::dontSendNotification);
    bpmInfoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    bpmInfoLabel.setJustificationType(juce::Justification::centredLeft);
    bpmInfoLabel.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    addAndMakeVisible(&bpmInfoLabel);
    DEBUG_LOG("PluginEditor: bpmInfoLabel added - STEP 6.8");
    
    // 啟動定時器，每 20ms 更新一次時間碼和回放（更頻繁的更新以確保回放流暢）
    startTimer(20);

    // 添加 JYPad 編輯器
    DEBUG_LOG("PluginEditor: Adding JYPadEditor to view - STEP 9");
    addAndMakeVisible (&jyPadEditor);
    DEBUG_LOG("PluginEditor: JYPadEditor added - STEP 10");
    
    // 設定輸出標籤
    DEBUG_LOG("PluginEditor: Setting up outputLabel - STEP 11");
    outputLabel.setText ("OSC Message Window:", juce::dontSendNotification);
    outputLabel.setColour (juce::Label::textColourId, juce::Colours::white);
    outputLabel.setJustificationType (juce::Justification::centredLeft);
    outputLabel.setFont(juce::Font(juce::FontOptions().withHeight(14.0f)));
    addAndMakeVisible (&outputLabel);
    DEBUG_LOG("PluginEditor: outputLabel added - STEP 12");
    
    // 設定輸出文字區域
    DEBUG_LOG("PluginEditor: Setting up outputText - STEP 13");
    outputText.setMultiLine (true);
    outputText.setReadOnly (true);
    outputText.setFont (juce::Font(juce::FontOptions().withHeight(12.0f)));
    outputText.setScrollbarsShown(true);
    outputText.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff2a2a2a));
    outputText.setColour (juce::TextEditor::textColourId, juce::Colours::white);
    addAndMakeVisible (&outputText);
    DEBUG_LOG("PluginEditor: outputText added - STEP 14");
    // 設定 JYPad 的回調，當球移動時發送 OSC（OSC 訊息會自動記錄到訊息視窗）
    DEBUG_LOG("PluginEditor: Setting up JYPad callback - STEP 15");
    audioProcessor.jyPad.onBallMoved = [this](int ballId, float x, float y) {
        // 座標乘以 10 用於顯示和輸出
        float outputX = x * 10.0f;
        float outputY = y * 10.0f;
        
        // 發送 OSC 訊息（使用乘以 10 後的座標，訊息會自動記錄到訊息視窗）
        if (audioProcessor.oscSettings.enabled)
        {
            audioProcessor.sendOSCMessage(ballId, outputX, outputY);
        }
    };
    // OSC Data 視窗按鈕（右上角，美化樣式）
    DEBUG_LOG("PluginEditor: Setting up OSC Data button - STEP 17");
    openOSCDataButton.setButtonText("OSC DATA");
    openOSCDataButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a90e2));
    openOSCDataButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openOSCDataButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff5aa0f2));
    openOSCDataButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    openOSCDataButton.onClick = [this] {
        if (oscDataWindow == nullptr)
        {
            oscDataWindow = std::make_unique<OSCDataWindow>(audioProcessor);
            oscDataWindow->setVisible(true);
        }
        else if (!oscDataWindow->isVisible())
        {
            oscDataWindow->setVisible(true);
        }
        else
        {
            oscDataWindow->toFront(true);
        }
    };
    openOSCDataButton.setVisible(false);
    addChildComponent(&openOSCDataButton);
    DEBUG_LOG("PluginEditor: OSC Data button added - STEP 18");
    
    // Network Settings 視窗按鈕（右上角，美化樣式）
    DEBUG_LOG("PluginEditor: Setting up Network Settings button - STEP 19");
    openNetworkSettingsButton.setButtonText("NETWORK");
    openNetworkSettingsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff50c878));
    openNetworkSettingsButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openNetworkSettingsButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff60d888));
    openNetworkSettingsButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    openNetworkSettingsButton.onClick = [this] {
        if (networkSettingsWindow == nullptr)
        {
            networkSettingsWindow = std::make_unique<NetworkSettingsWindow>(audioProcessor);
            networkSettingsWindow->setVisible(true);
        }
        else if (!networkSettingsWindow->isVisible())
        {
            networkSettingsWindow->setVisible(true);
        }
        else
        {
            networkSettingsWindow->toFront(true);
        }
    };
    addAndMakeVisible(&openNetworkSettingsButton);
    DEBUG_LOG("PluginEditor: Network Settings button added - STEP 20");
    
        DEBUG_LOG("PluginEditor: All UI components initialized - STEP 21");
        DEBUG_LOG("PluginEditor: Constructor completed successfully - FINAL");
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("PluginEditor: Exception in constructor: " + juce::String(e.what()));
        throw;
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("PluginEditor: Unknown exception in constructor");
        throw;
    }
}

PlugDataCustomObjectAudioProcessorEditor::~PlugDataCustomObjectAudioProcessorEditor()
{
    DEBUG_LOG("PluginEditor: Destructor called");
    try
    {
        stopTimer();
        openOSCDataButton.setLookAndFeel(nullptr);
        openNetworkSettingsButton.setLookAndFeel(nullptr);
        
        // 關閉所有子視窗
        if (oscDataWindow != nullptr)
        {
            oscDataWindow->setVisible(false);
            oscDataWindow = nullptr;
        }
        if (networkSettingsWindow != nullptr)
        {
            networkSettingsWindow->setVisible(false);
            networkSettingsWindow = nullptr;
        }
        
        DEBUG_LOG("PluginEditor: Destructor completed");
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("PluginEditor: Exception in destructor: " + juce::String(e.what()));
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("PluginEditor: Unknown exception in destructor");
    }
}

//==============================================================================
void PlugDataCustomObjectAudioProcessorEditor::visibilityChanged()
{
    // 當 plugin 視窗被隱藏或關閉時，也關閉所有子視窗
    if (!isVisible())
    {
        if (oscDataWindow != nullptr && oscDataWindow->isVisible())
        {
            oscDataWindow->setVisible(false);
        }
        if (networkSettingsWindow != nullptr && networkSettingsWindow->isVisible())
        {
            networkSettingsWindow->setVisible(false);
        }
    }
}

//==============================================================================
void PlugDataCustomObjectAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 繪製背景
    g.fillAll (juce::Colour (0xff1e1e1e));

    // 繪製標題
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("JYPad", getLocalBounds().removeFromTop (50),
                      juce::Justification::centred, 1);
    auto timeCodeBox = getLocalBounds().reduced(20).removeFromTop(50).removeFromLeft(115);
    g.setColour(juce::Colour(0xff000000));  // 黑色背景
    g.fillRoundedRectangle(timeCodeBox.toFloat(), 4.0f);
    g.setColour(juce::Colour(0xff404040));  // 深灰色邊框
    g.drawRoundedRectangle(timeCodeBox.toFloat(), 4.0f, 1.5f);
}

void PlugDataCustomObjectAudioProcessorEditor::resized()
{
    // 設定元件位置
    auto area = getLocalBounds().reduced (20);
    
    // 時間碼區域（左上角，兩行顯示，放在最上方，帶黑底方框）
    auto timeCodeArea = area.removeFromTop(50).removeFromLeft(115);  // 增加寬度以容納更多位數
    // 在方框內留出內邊距
    auto timeCodeInnerArea = timeCodeArea.reduced(6, 6);
    midiTimeLabel.setBounds(timeCodeInnerArea.removeFromTop(19));  // MIDI time 在第一行
    timeCodeLabel.setBounds(timeCodeInnerArea.removeFromTop(19));  // Time code 在第二行
    
    // BPM 資訊（左側，與時間碼方框相鄰）
    auto bpmArea = getLocalBounds().reduced(20).removeFromTop(50);
    bpmArea.removeFromLeft(135);  // 跳過時間碼方框（115 + 20 間距）
    bpmArea = bpmArea.removeFromLeft(80);  // BPM 標籤寬度（縮短）
    bpmInfoLabel.setJustificationType(juce::Justification::centredLeft);  // 改為左對齊
    bpmInfoLabel.setBounds(bpmArea);
    
    // Network 按鈕（右上角，與時間碼/BPM 同一水平線）
    auto headerArea = getLocalBounds().reduced(20).removeFromTop(50);
    auto buttonArea = headerArea.removeFromRight(80);  // 從右側分配空間給按鈕
    openNetworkSettingsButton.setBounds(buttonArea.removeFromRight(70).reduced(2));  // 按鈕寬度 70px
    
    area.removeFromTop(10);  // 標題區域間距
    
    // JYPad 編輯器（正方形）
    int padSize = juce::jmin(area.getWidth() - 40, 400);
    auto padArea = area.removeFromTop(padSize).withSizeKeepingCentre(padSize, padSize);
    jyPadEditor.setBounds(padArea);
    
    area.removeFromTop(20);  // 間距
    
    // 輸出標籤和文字區域
    outputLabel.setBounds(area.removeFromTop(25));
    area.removeFromTop(5);
    outputText.setBounds(area.removeFromTop(100));
}

void PlugDataCustomObjectAudioProcessorEditor::timerCallback()
{
    // 更新時間碼顯示
    auto timeInfo = audioProcessor.getTimeCodeInfo();
    
    if (timeInfo.isValid)
    {
        // 檢測播放狀態變化：從播放變為停止
        bool isPlayingChanged = (wasPlaying != timeInfo.isPlaying);
        wasPlaying = timeInfo.isPlaying;
        
        // 如果從播放變為停止，重置所有球到第一個錄製事件的位置
        if (isPlayingChanged && !timeInfo.isPlaying)
        {
            bool needsRepaint = false;
            for (const auto& ball : audioProcessor.jyPad.getAllBalls())
            {
                const RecordedEvent* firstEvent = audioProcessor.jyPad.getFirstEvent(ball.id);
                if (firstEvent != nullptr)
                {
                    // 有錄製數據，重置到第一個事件的位置
                    audioProcessor.jyPad.setBallPosition(ball.id, firstEvent->x, firstEvent->y);
                    needsRepaint = true;
                }
                else
                {
                    // 沒有錄製數據，重置到中心 (0, 0)
                    audioProcessor.jyPad.setBallPosition(ball.id, 0.0f, 0.0f);
                    needsRepaint = true;
                }
            }
            if (needsRepaint)
            {
                jyPadEditor.repaint();
            }
        }
        
        // 當不在播放狀態時，如果 MIDI time 改變，更新球的位置到該時間點之前最後一個事件
        if (!timeInfo.isPlaying && timeInfo.isValid)
        {
            if (std::abs(timeInfo.ppqPosition - lastMidiTime) > 0.001)  // MIDI time 改變了
            {
                // 遍歷所有球，更新它們的位置
                const auto& balls = audioProcessor.jyPad.getAllBalls();
                bool needsUpdate = false;
                for (const auto& ball : balls)
                {
                    if (!ball.isRecording)  // 只在非錄製狀態下更新
                    {
                        const auto* event = audioProcessor.jyPad.getLastEventBeforeTime(ball.id, timeInfo.ppqPosition);
                        if (event != nullptr)
                        {
                            // 更新球的位置到該事件的位置
                            audioProcessor.jyPad.setBallPosition(ball.id, event->x, event->y);
                            needsUpdate = true;
                        }
                    }
                }
                if (needsUpdate)
                {
                    jyPadEditor.repaint();
                }
            }
            lastMidiTime = timeInfo.ppqPosition;
        }
        else if (timeInfo.isPlaying)
        {
            // 播放狀態下也更新 lastMidiTime，以便在停止時能正確檢測變化
            lastMidiTime = timeInfo.ppqPosition;
        }
        
        // 第一行：MIDI Time (Bar.Beat.Ticks)
        juce::String midiTimeStr = formatMIDITime(timeInfo.ppqPosition, timeInfo.bpm);
        midiTimeLabel.setText(midiTimeStr, juce::dontSendNotification);
        
        // 第二行：Time Code (m:ss.SSS)
        juce::String timeCodeStr = formatTimeCode(timeInfo.timeInSeconds);
        timeCodeLabel.setText(timeCodeStr, juce::dontSendNotification);
        
        // 右側：BPM 資訊
        juce::String bpmInfoStr = formatBPMInfo(timeInfo.bpm, timeInfo.isPlaying);
        bpmInfoLabel.setText(bpmInfoStr, juce::dontSendNotification);
        
        // 根據播放狀態改變顏色
        juce::Colour activeColor = timeInfo.isPlaying ? juce::Colour(0xff00ff00) : juce::Colour(0xff00aa00);
        juce::Colour inactiveColor = juce::Colour(0xff666666);
        
        timeCodeLabel.setColour(juce::Label::textColourId, activeColor);
        midiTimeLabel.setColour(juce::Label::textColourId, activeColor);
        bpmInfoLabel.setColour(juce::Label::textColourId, 
                               timeInfo.isPlaying ? juce::Colours::lightgreen : juce::Colours::lightgrey);
        
        // 如果有球在 recording 狀態，需要持續更新視圖以顯示閃爍效果
        bool hasRecordingBall = false;
        bool needsRepaint = false;
        
        // 檢查所有球是否需要回放錄製的事件（只在播放狀態時回放）
        if (timeInfo.isPlaying)
        {
            for (const auto& ball : audioProcessor.jyPad.getAllBalls())
            {
                if (ball.isRecording)
                {
                    hasRecordingBall = true;
                }
                
                // 檢查是否有錄製的事件需要回放（只在不在 recording 狀態時回放，避免與手動拖動衝突）
                if (!ball.isRecording)  // 只在不在 recording 狀態時回放
                {
                    const RecordedEvent* event = audioProcessor.jyPad.getEventAtTime(ball.id, timeInfo.ppqPosition);
                    if (event != nullptr)
                    {
                        // 更新球的位置（只在位置改變時更新，節省資源）
                        audioProcessor.jyPad.setBallPosition(ball.id, event->x, event->y);
                        needsRepaint = true;
                    }
                }
            }
        }
        
        if (hasRecordingBall || needsRepaint)
        {
            jyPadEditor.repaint();
        }
    }
    else
    {
        midiTimeLabel.setText("1.1.000", juce::dontSendNotification);
        timeCodeLabel.setText("0:00.000", juce::dontSendNotification);
        bpmInfoLabel.setText("-- BPM", juce::dontSendNotification);
        
        midiTimeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff333333));
        timeCodeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff333333));
        bpmInfoLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    }
}

//==============================================================================
juce::String PlugDataCustomObjectAudioProcessorEditor::formatTimeCode(double timeInSeconds) const
{
    // 格式化時間碼：m:ss.SSS（分鐘:秒.毫秒，分鐘不補零，自動擴展）
    int totalMilliseconds = static_cast<int>(timeInSeconds * 1000.0);
    int minutes = totalMilliseconds / 60000;
    int seconds = (totalMilliseconds / 1000) % 60;
    int milliseconds = totalMilliseconds % 1000;
    
    // 格式：m:ss.SSS（分鐘不補零，可以超過兩位數）
    return juce::String::formatted("%d:%02d.%03d", minutes, seconds, milliseconds);
}

juce::String PlugDataCustomObjectAudioProcessorEditor::formatMIDITime(double ppqPosition, double bpm) const
{
    if (ppqPosition < 0.0 || bpm <= 0.0)
        return "1.1.000";
    
    // 獲取時間簽名資訊
    auto timeInfo = audioProcessor.getTimeCodeInfo();
    int numerator = timeInfo.timeSignatureNumerator;
    int denominator = timeInfo.timeSignatureDenominator;
    
    if (numerator == 0 || denominator == 0)
    {
        numerator = 4;
        denominator = 4;
    }
    
    // 標準 MIDI 時間格式：Bar.Beat.Ticks
    // 960 ticks per quarter note (標準 MIDI 解析度)
    const double ticksPerQuarter = 960.0;
    double quarterNotesPerBar = (numerator * 4.0) / denominator;
    
    // 計算 bar, beat, tick
    // 使用 ppqPositionOfLastBarStart 來計算當前 bar
    double ppqFromBarStart = ppqPosition - timeInfo.ppqPositionOfLastBarStart;
    if (ppqFromBarStart < 0.0)
        ppqFromBarStart = ppqPosition;  // 如果沒有 bar start 資訊，使用絕對位置
    
    int bars = static_cast<int>(ppqPosition / quarterNotesPerBar) + 1;  // Bar 從 1 開始
    double ppqInBar = std::fmod(ppqFromBarStart, quarterNotesPerBar);
    if (ppqInBar < 0.0)
        ppqInBar += quarterNotesPerBar;
    
    // 計算 beat（根據 time signature）
    double beatsInBar = (ppqInBar / quarterNotesPerBar) * numerator;
    int beat = static_cast<int>(beatsInBar) + 1;  // Beat 從 1 開始
    
    // 計算 ticks（在當前 beat 內的 ticks）
    double quarterNotesPerBeat = quarterNotesPerBar / numerator;
    double ppqInBeat = std::fmod(ppqInBar, quarterNotesPerBeat);
    int ticks = static_cast<int>(ppqInBeat * ticksPerQuarter);
    
    // 確保值在合理範圍內
    bars = juce::jmax(1, bars);
    beat = juce::jlimit(1, numerator, beat);
    ticks = juce::jlimit(0, static_cast<int>(quarterNotesPerBeat * ticksPerQuarter) - 1, ticks);
    
    // 格式：Bar.Beat.Ticks（不補零，可以超過兩位數）
    return juce::String::formatted("%d.%d.%03d", bars, beat, ticks);
}

juce::String PlugDataCustomObjectAudioProcessorEditor::formatBPMInfo(double bpm, bool isPlaying) const
{
    if (bpm <= 0.0)
        return "-- BPM";
    
    juce::String info = juce::String::formatted("%.1f BPM", bpm);
    
    if (isPlaying)
        info += " ▶";
    else
        info += " ⏸";
    
    return info;
}

//==============================================================================
void PlugDataCustomObjectAudioProcessorEditor::logOSCMessage(const juce::String& message)
{
    // 在訊息線程中更新 OSC 訊息視窗
    if (juce::MessageManager::getInstance()->isThisTheMessageThread())
    {
        juce::String currentText = outputText.getText();
        juce::StringArray lines;
        lines.addTokens(currentText, "\n", "");
        
        // 添加新訊息
        lines.add(message);
        
        // 只保留最後 100 行
        while (lines.size() > 100)
        {
            lines.remove(0);
        }
        
        // 更新文字並自動捲動到底部
        juce::String newText = lines.joinIntoString("\n");
        outputText.setText(newText, juce::dontSendNotification);
        outputText.moveCaretToEnd();
    }
    else
    {
        // 如果不在訊息線程中，使用異步更新
        juce::MessageManager::callAsync([this, message]() {
            logOSCMessage(message);
        });
    }
}
