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
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    
    // 滾輪和觸控板手勢（縮放）
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    // 觸控手勢（兩指縮放）
    void mouseMagnify(const juce::MouseEvent& e, float scaleFactor) override;
    
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
    
    // 縮放比例（用於兩指縮放）
    // 注意：實際值存儲在 audioProcessor.zoomScale 中
    float& zoomScale;
    
    // 將螢幕座標轉換為邏輯座標
    juce::Point<float> screenToLogic(juce::Point<int> screenPos) const;
    
    // 檢查點擊是否在球上
    int getBallAtPosition(juce::Point<int> pos) const;
    
    // 繪製網格
    void drawGrid(juce::Graphics& g);
    
    // 繪製參考圓（中心圓和半徑圓）
    void drawReferenceCircles(juce::Graphics& g);
    
    // 繪製球體
    void drawBalls(juce::Graphics& g);
    
    // 將邏輯座標轉換為螢幕座標（考慮縮放）
    juce::Point<float> logicToScreen(float logicX, float logicY) const;
    
    // 將螢幕座標轉換為邏輯座標（考慮縮放）
    juce::Point<float> screenToLogicWithZoom(juce::Point<int> screenPos) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JYPadEditor)
};

