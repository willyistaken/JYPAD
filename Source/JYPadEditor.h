#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "JYPad.h"
#include "SourceEditWindow.h"
#include "PluginProcessor.h"

//==============================================================================
/**
 * JYPad 視覺化編輯器
 * 顯示 2D 平面和可拖動的圓球
 */
class JYPadEditor : public juce::Component
{
public:
    JYPadEditor(JYPad& pad, PlugDataCustomObjectAudioProcessor& processor);
    ~JYPadEditor() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // 滑鼠事件
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    // 右鍵選單
    void showContextMenu(juce::Point<int> screenPosition, int ballId, juce::Point<int> localPosition);
    void showAddSourceMenu(juce::Point<int> localPosition);
    void showEditSourceMenu(int ballId);
    void showDeleteConfirmMenu(int ballId);

    // 更新顯示
    void updateDisplay();

private:
    JYPad& jyPad;
    PlugDataCustomObjectAudioProcessor& audioProcessor;
    
    // 當前拖動的球（如果有的話）
    int draggedBallId = -1;
    
    // 球體視覺大小
    static constexpr float ballRadius = 12.0f;
    
    // 將螢幕座標轉換為邏輯座標
    juce::Point<float> screenToLogic(juce::Point<int> screenPos) const;
    
    // 檢查點擊是否在球上
    int getBallAtPosition(juce::Point<int> pos) const;
    
    // 繪製網格
    void drawGrid(juce::Graphics& g);
    
    // 繪製球體
    void drawBalls(juce::Graphics& g);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JYPadEditor)
};

