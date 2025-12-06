#include "JYPadEditor.h"
#include "PluginProcessor.h"
#include "DebugLogger.h"
#include <cmath>

//==============================================================================
JYPadEditor::JYPadEditor(JYPad& pad, PlugDataCustomObjectAudioProcessor& processor)
    : jyPad(pad), audioProcessor(processor), zoomScale(processor.zoomScale)
{
    DEBUG_LOG("JYPadEditor: Constructor started - XY_STEP 1");
    try
    {
        DEBUG_LOG("JYPadEditor: Setting mouse cursor - XY_STEP 2");
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
        DEBUG_LOG("JYPadEditor: Mouse cursor set - XY_STEP 3");
        
        // 設定回調，當球移動時更新顯示（用於非拖曳情況，如程式化更新）
        DEBUG_LOG("JYPadEditor: Setting up onBallMoved callback - XY_STEP 4");
        jyPad.onBallMoved = [this]([[maybe_unused]] int ballId, [[maybe_unused]] float x, [[maybe_unused]] float y) {
            // 如果不在拖曳狀態，才通過回調更新（拖曳時已經直接調用 repaint）
            if (draggedBallId < 0)
            {
                if (juce::MessageManager::getInstance()->isThisTheMessageThread())
                {
                    repaint();
                }
                else
                {
                    juce::MessageManager::callAsync([this]() {
                        repaint();
                    });
                }
            }
        };
        DEBUG_LOG("JYPadEditor: Callback set - XY_STEP 5");
        
        DEBUG_LOG("JYPadEditor: Constructor completed successfully - XY_FINAL");
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("JYPadEditor: Exception in constructor: " + juce::String(e.what()));
        throw;
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("JYPadEditor: Unknown exception in constructor");
        throw;
    }
}

JYPadEditor::~JYPadEditor()
{
}

//==============================================================================
void JYPadEditor::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // 繪製背景
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRoundedRectangle(bounds, 4.0f);
    
    // 繪製邊框
    g.setColour(juce::Colour(0xff404040));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
    
    // 繪製網格
    drawGrid(g);
    
    // 繪製參考圓（中心圓和半徑圓）
    drawReferenceCircles(g);
    
    // 繪製球體
    drawBalls(g);
}

void JYPadEditor::drawGrid(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    g.setColour(juce::Colour(0xff2a2a2a));
    
    // 繪製中心線（X 和 Y 軸）
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    
    // 垂直中心線
    g.drawLine(centerX, bounds.getY(), centerX, bounds.getBottom(), 1.0f);
    
    // 水平中心線
    g.drawLine(bounds.getX(), centerY, bounds.getRight(), centerY, 1.0f);
    
    // 繪製四等分線
    float quarterX1 = bounds.getX() + bounds.getWidth() * 0.25f;
    float quarterX2 = bounds.getX() + bounds.getWidth() * 0.75f;
    float quarterY1 = bounds.getY() + bounds.getHeight() * 0.25f;
    float quarterY2 = bounds.getY() + bounds.getHeight() * 0.75f;
    
    g.setColour(juce::Colour(0xff252525));
    g.drawLine(quarterX1, bounds.getY(), quarterX1, bounds.getBottom(), 0.5f);
    g.drawLine(quarterX2, bounds.getY(), quarterX2, bounds.getBottom(), 0.5f);
    g.drawLine(bounds.getX(), quarterY1, bounds.getRight(), quarterY1, 0.5f);
    g.drawLine(bounds.getX(), quarterY2, bounds.getRight(), quarterY2, 0.5f);
    
    // 繪製中心點標記
    g.setColour(juce::Colour(0xff505050));
    g.fillEllipse(centerX - 2.0f, centerY - 2.0f, 4.0f, 4.0f);
}

void JYPadEditor::drawReferenceCircles(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    
    // 計算縮放後的尺寸
    float padWidth = bounds.getWidth() * zoomScale;
    float padHeight = bounds.getHeight() * zoomScale;
    
    // 根據縮放比例計算顯示範圍
    // zoomScale = 0.1（最小縮放）時，顯示範圍 = -20 到 20（更大的範圍）
    // zoomScale = 10.0（最大縮放）時，顯示範圍 = -0.5 到 0.5（更小的範圍）
    float displayRange = 20.0f - (zoomScale - 0.1f) * 19.5f / 9.9f;
    displayRange = juce::jlimit(0.5f, 20.0f, displayRange);
    
    // 邏輯座標到螢幕座標的轉換比例（考慮縮放和顯示範圍）
    float scaleX = padWidth / (displayRange * 2.0f);
    float scaleY = padHeight / (displayRange * 2.0f);
    float scale = juce::jmin(scaleX, scaleY);  // 使用較小的 scale 以保持圓形
    
    g.setColour(juce::Colour(0xff404040).withAlpha(0.5f));
    
    // 從 0.1 到 3.0，每隔 0.1 畫一個圓（但只在顯示範圍內繪製）
    for (float radius = 0.1f; radius <= 3.0f; radius += 0.1f)
    {
        // 只繪製在顯示範圍內的圓
        if (radius <= displayRange)
        {
            // 將邏輯座標半徑轉換為螢幕座標
            float screenRadius = radius * scale;
            
            // 繪製圓（線寬固定為 1）
            g.drawEllipse(centerX - screenRadius, centerY - screenRadius, 
                         screenRadius * 2.0f, screenRadius * 2.0f, 
                         1.0f);
        }
    }
    
    // 繪製中心點（半徑 1 像素的實心圓）
    g.setColour(juce::Colour(0xff606060));
    g.fillEllipse(centerX - 1.0f, centerY - 1.0f, 2.0f, 2.0f);
}

void JYPadEditor::drawBalls(juce::Graphics& g)
{
    const auto& balls = jyPad.getAllBalls();
    
    for (const auto& ball : balls)
    {
        // 將邏輯座標轉換為螢幕座標（考慮縮放）
        auto screenPos = logicToScreen(ball.x, ball.y);
        float screenX = screenPos.x;
        float screenY = screenPos.y;
        
        // 計算透明度：如果 mute 則為 20%，否則為 100%
        // 安全檢查 isMuted（防止未初始化）
        bool isMuted = ball.isMuted;
        float alpha = isMuted ? 0.2f : 1.0f;
        
        // 繪製球體陰影
        g.setColour(juce::Colour(0x40000000).withAlpha(alpha));
        g.fillEllipse(screenX - ballRadius + 1.0f, screenY - ballRadius + 1.0f, 
                     ballRadius * 2.0f, ballRadius * 2.0f);
        
        // 繪製球體（使用 Ball 的 color 欄位，應用透明度）
        g.setColour(ball.color.withAlpha(alpha));
        g.fillEllipse(screenX - ballRadius, screenY - ballRadius, 
                     ballRadius * 2.0f, ballRadius * 2.0f);
        
        // 繪製球體邊框
        g.setColour(juce::Colours::white.withAlpha(0.3f * alpha));
        g.drawEllipse(screenX - ballRadius, screenY - ballRadius, 
                     ballRadius * 2.0f, ballRadius * 2.0f, 1.5f);
        
        // 繪製球體編號
        g.setColour(juce::Colours::white.withAlpha(alpha));
        g.setFont(10.0f);
        g.drawText(juce::String(ball.id), 
                   static_cast<int>(screenX - ballRadius), 
                   static_cast<int>(screenY - ballRadius), 
                   static_cast<int>(ballRadius * 2.0f), 
                   static_cast<int>(ballRadius * 2.0f),
                   juce::Justification::centred);
        
        // 如果 solo，繪製一個標記
        // 安全檢查 isSoloed（防止未初始化）
        bool isSoloed = ball.isSoloed;
        if (isSoloed)
        {
            g.setColour(juce::Colours::yellow.withAlpha(alpha));
            g.fillEllipse(screenX - ballRadius - 3.0f, screenY - ballRadius - 3.0f, 6.0f, 6.0f);
        }
        
        // 如果 recording，繪製閃爍的紅圈
        // 安全檢查 isRecording（防止未初始化）
        bool isRecording = ball.isRecording;
        if (isRecording)
        {
            // 獲取當前的時間碼資訊來計算 ticks
            auto timeInfo = audioProcessor.getTimeCodeInfo();
            int ticks = 0;
            
            if (timeInfo.isValid && timeInfo.bpm > 0.0)
            {
                // 計算當前的 ticks（在當前 beat 內的 ticks）
                int numerator = timeInfo.timeSignatureNumerator;
                int denominator = timeInfo.timeSignatureDenominator;
                
                if (numerator == 0 || denominator == 0)
                {
                    numerator = 4;
                    denominator = 4;
                }
                
                const double ticksPerQuarter = 960.0;
                double quarterNotesPerBar = (numerator * 4.0) / denominator;
                double ppqFromBarStart = timeInfo.ppqPosition - timeInfo.ppqPositionOfLastBarStart;
                if (ppqFromBarStart < 0.0)
                    ppqFromBarStart = timeInfo.ppqPosition;
                
                double ppqInBar = std::fmod(ppqFromBarStart, quarterNotesPerBar);
                if (ppqInBar < 0.0)
                    ppqInBar += quarterNotesPerBar;
                
                double quarterNotesPerBeat = quarterNotesPerBar / numerator;
                double ppqInBeat = std::fmod(ppqInBar, quarterNotesPerBeat);
                ticks = static_cast<int>(ppqInBeat * ticksPerQuarter);
                
                // 限制 ticks 在 0-999 範圍內
                ticks = juce::jlimit(0, 999, ticks);
            }
            
            // 計算亮度：ticks=0 時 100%，ticks=999 時 50%
            // 線性插值：brightness = 1.0 - (ticks / 999.0) * 0.5
            float brightness = 1.0f - (static_cast<float>(ticks) / 999.0f) * 0.5f;
            brightness = juce::jlimit(0.5f, 1.0f, brightness);
            
            // 繪製閃爍的紅圈（在球外面）
            float ringRadius = ballRadius + 5.0f;  // 紅圈在球外面 5 像素
            float ringThickness = 1.5f;  // 更細的線條
            
            g.setColour(juce::Colour(0xffff0000).withAlpha(brightness * alpha));
            g.drawEllipse(screenX - ringRadius, screenY - ringRadius, 
                         ringRadius * 2.0f, ringRadius * 2.0f, ringThickness);
        }
    }
}

//==============================================================================
void JYPadEditor::resized()
{
    repaint();
}

//==============================================================================
void JYPadEditor::mouseDown(const juce::MouseEvent& e)
{
    // 右鍵選單
    if (e.mods.isRightButtonDown())
    {
        int ballId = getBallAtPosition(e.getPosition());
        juce::Point<int> screenPos = localPointToGlobal(e.getPosition());
        showContextMenu(screenPos, ballId, e.getPosition());
        return;
    }
    
    // 檢查是否點擊在球上
    draggedBallId = getBallAtPosition(e.getPosition());
    
    if (draggedBallId >= 0)
    {
        // 檢查球是否被 mute，如果是則不允許移動
        auto* ball = jyPad.getBall(draggedBallId);
        if (ball != nullptr)
        {
            // 安全檢查 isMuted（防止未初始化）
            bool isMuted = ball->isMuted;
            if (isMuted)
            {
                draggedBallId = -1;  // 不允許拖動
                return;
            }
        }
        
        // 只有點擊在球上且未被 mute 才開始拖動
        auto logicPos = screenToLogicWithZoom(e.getPosition());
        jyPad.setBallPosition(draggedBallId, logicPos.x, logicPos.y);
        // 立即重繪
        repaint();
    }
    // 如果點擊在空白處，不做任何操作
}

void JYPadEditor::mouseDrag(const juce::MouseEvent& e)
{
    if (draggedBallId >= 0)
    {
        // 再次檢查球是否被 mute（防止在拖動過程中狀態改變）
        auto* ball = jyPad.getBall(draggedBallId);
        if (ball != nullptr)
        {
            // 安全檢查 isMuted（防止未初始化）
            bool isMuted = ball->isMuted;
            if (isMuted)
            {
                draggedBallId = -1;  // 停止拖動
                return;
            }
            
            // 如果球處於 recording 狀態，記錄事件
            bool isRecording = ball->isRecording;
            if (isRecording)
            {
                // 獲取當前的 MIDI time
                auto timeInfo = audioProcessor.getTimeCodeInfo();
                if (timeInfo.isValid)
                {
                    auto logicPos = screenToLogicWithZoom(e.getPosition());
                    // 記錄事件：ID, MIDI time, x, y, z (z 暫時為 0)
                    jyPad.recordEvent(draggedBallId, timeInfo.ppqPosition, 
                                      logicPos.x, logicPos.y, 0.0f);
                }
            }
        }
        
        auto logicPos = screenToLogicWithZoom(e.getPosition());
        jyPad.setBallPosition(draggedBallId, logicPos.x, logicPos.y);
        // 立即重繪以確保視覺更新
        repaint();
    }
}

void JYPadEditor::mouseUp([[maybe_unused]] const juce::MouseEvent& e)
{
    draggedBallId = -1;
}

void JYPadEditor::mouseDoubleClick(const juce::MouseEvent& e)
{
    // 檢查雙擊是否在球上
    int ballId = getBallAtPosition(e.getPosition());
    if (ballId >= 0)
    {
        // 切換該球的 recording 狀態
        auto* ball = jyPad.getBall(ballId);
        if (ball != nullptr)
        {
            ball->isRecording = !ball->isRecording;
            repaint();
        }
    }
}

//==============================================================================
juce::Point<float> JYPadEditor::screenToLogic(juce::Point<int> screenPos) const
{
    return screenToLogicWithZoom(screenPos);
}

juce::Point<float> JYPadEditor::screenToLogicWithZoom(juce::Point<int> screenPos) const
{
    auto bounds = getLocalBounds().toFloat();
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    
    // 計算縮放後的 pad 尺寸和位置
    float padWidth = bounds.getWidth() * zoomScale;
    float padHeight = bounds.getHeight() * zoomScale;
    float padX = centerX - padWidth * 0.5f;
    float padY = centerY - padHeight * 0.5f;
    
    // 根據縮放比例計算顯示範圍
    // zoomScale = 0.1（最小縮放）時，顯示範圍 = -20 到 20（更大的範圍）
    // zoomScale = 10.0（最大縮放）時，顯示範圍 = -0.5 到 0.5（更小的範圍）
    float displayRange = 20.0f - (zoomScale - 0.1f) * 19.5f / 9.9f;
    displayRange = juce::jlimit(0.5f, 20.0f, displayRange);
    
    // 將螢幕座標轉換為相對於縮放後 pad 的座標
    float relativeX = (screenPos.x - padX) / padWidth;
    float relativeY = (screenPos.y - padY) / padHeight;
    
    // 轉換為邏輯座標（根據顯示範圍）
    float logicX = (relativeX * 2.0f - 1.0f) * displayRange;
    float logicY = (1.0f - relativeY * 2.0f) * displayRange;  // 翻轉 Y 軸
    
    return juce::Point<float>(logicX, logicY);
}

juce::Point<float> JYPadEditor::logicToScreen(float logicX, float logicY) const
{
    auto bounds = getLocalBounds().toFloat();
    float centerX = bounds.getCentreX();
    float centerY = bounds.getCentreY();
    
    // 計算縮放後的 pad 尺寸和位置
    float padWidth = bounds.getWidth() * zoomScale;
    float padHeight = bounds.getHeight() * zoomScale;
    float padX = centerX - padWidth * 0.5f;
    float padY = centerY - padHeight * 0.5f;
    
    // 根據縮放比例計算顯示範圍
    // zoomScale = 0.1（最小縮放）時，顯示範圍 = -20 到 20（更大的範圍）
    // zoomScale = 10.0（最大縮放）時，顯示範圍 = -0.5 到 0.5（更小的範圍）
    float displayRange = 20.0f - (zoomScale - 0.1f) * 19.5f / 9.9f;
    displayRange = juce::jlimit(0.5f, 20.0f, displayRange);
    
    // 將邏輯座標轉換為相對座標 (0 to 1)
    float normalizedX = logicX / displayRange;
    float normalizedY = logicY / displayRange;
    float relativeX = (normalizedX + 1.0f) * 0.5f;
    float relativeY = (1.0f - normalizedY) * 0.5f;  // 翻轉 Y 軸
    
    // 轉換為螢幕座標
    float screenX = padX + relativeX * padWidth;
    float screenY = padY + relativeY * padHeight;
    
    return juce::Point<float>(screenX, screenY);
}

int JYPadEditor::getBallAtPosition(juce::Point<int> pos) const
{
    const auto& balls = jyPad.getAllBalls();
    
    for (const auto& ball : balls)
    {
        // 將邏輯座標轉換為螢幕座標（考慮縮放）
        auto screenPos = logicToScreen(ball.x, ball.y);
        
        // 檢查距離
        float dx = pos.x - screenPos.x;
        float dy = pos.y - screenPos.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance <= ballRadius)
            return ball.id;
    }
    
    return -1;
}

void JYPadEditor::updateDisplay()
{
    repaint();
}

//==============================================================================
// 兩指縮放手勢處理
void JYPadEditor::mouseMagnify([[maybe_unused]] const juce::MouseEvent& e, float scaleFactor)
{
    // scaleFactor > 1.0 表示放大，< 1.0 表示縮小
    // 累積縮放比例
    zoomScale *= scaleFactor;
    
    // 限制縮放範圍（0.1x 到 10x）
    zoomScale = juce::jlimit(0.1f, 10.0f, zoomScale);
    
    repaint();
}

//==============================================================================
void JYPadEditor::showContextMenu(juce::Point<int> screenPosition, int ballId, juce::Point<int> localPosition)
{
    juce::PopupMenu menu;
    
    if (ballId >= 0)
    {
        auto* ball = jyPad.getBall(ballId);
        if (ball != nullptr)
        {
            // 在球上右鍵：顯示 Edit、Delete、Mute、Solo、Recording
            menu.addItem(1, "Edit");
            menu.addItem(2, "Delete");
            menu.addSeparator();
            menu.addItem(4, "Mute", true, ball->isMuted);
            menu.addItem(5, "Solo", true, ball->isSoloed);
            menu.addItem(6, "Recording", true, ball->isRecording);
            menu.addSeparator();
            // 新功能 section
            menu.addItem(7, "Clear Events");
            menu.addItem(8, "Set Position");
            menu.addItem(9, "Tween to next");
        }
    }
    else
    {
        // 在空白處右鍵：顯示 Add Source
        menu.addItem(3, "Add Source");
    }
    
    // 使用 withMousePosition() 確保選單在滑鼠位置顯示
    menu.showMenuAsync(juce::PopupMenu::Options()
                       .withTargetComponent(this)
                       .withMousePosition(),
                       [this, ballId, localPosition](int result)
                       {
                           if (result == 1)
                           {
                               // Edit
                               showEditSourceMenu(ballId);
                           }
                           else if (result == 2)
                           {
                               // Delete
                               showDeleteConfirmMenu(ballId);
                           }
                           else if (result == 3)
                           {
                               // Add Source
                               showAddSourceMenu(localPosition);
                           }
                           else if (result == 4)
                           {
                               // Toggle Mute
                               auto* ball = jyPad.getBall(ballId);
                               if (ball != nullptr)
                               {
                                   ball->isMuted = !ball->isMuted;
                                   repaint();
                               }
                           }
                           else if (result == 5)
                           {
                               // Toggle Solo
                               auto* ball = jyPad.getBall(ballId);
                               if (ball != nullptr)
                               {
                                   ball->isSoloed = !ball->isSoloed;
                                   repaint();
                               }
                           }
                           else if (result == 6)
                           {
                               // Toggle Recording
                               auto* ball = jyPad.getBall(ballId);
                               if (ball != nullptr)
                               {
                                   ball->isRecording = !ball->isRecording;
                                   repaint();
                               }
                           }
                           else if (result == 7)
                           {
                               // Clear Events - 清除這個球的所有路徑資料
                               jyPad.clearRecordedEvents(ballId);
                               repaint();
                           }
                           else if (result == 8)
                           {
                               // Set Position - 在當前時間點插入現在位置資料
                               auto* ball = jyPad.getBall(ballId);
                               if (ball != nullptr)
                               {
                                   auto timeInfo = audioProcessor.getTimeCodeInfo();
                                   if (timeInfo.isValid)
                                   {
                                       jyPad.insertEventAtTime(ballId, timeInfo.ppqPosition, 
                                                               ball->x, ball->y, 0.0f);
                                       repaint();
                                   }
                               }
                           }
                           else if (result == 9)
                           {
                               // Tween to next - 產生到下一個位置中間的差值（根據錄製頻率）
                               auto timeInfo = audioProcessor.getTimeCodeInfo();
                               if (timeInfo.isValid)
                               {
                                   jyPad.tweenToNext(ballId, timeInfo.ppqPosition, 
                                                     timeInfo.bpm, 
                                                     timeInfo.timeSignatureNumerator, 
                                                     timeInfo.timeSignatureDenominator);
                                   repaint();
                               }
                           }
                       });
}

void JYPadEditor::showAddSourceMenu(juce::Point<int> localPosition)
{
    // 計算下一個 source number
    int nextNumber = 1;
    const auto& balls = jyPad.getAllBalls();
    for (const auto& ball : balls)
    {
        if (ball.sourceNumber >= nextNumber)
            nextNumber = ball.sourceNumber + 1;
    }
    
    SourceEditWindow::SourceInfo info;
    info.oscPrefix = "/jypad/ball";
    info.color = juce::Colour(0xff4a90e2);
    info.sourceName = "Source " + juce::String(nextNumber);
    info.sourceNumber = nextNumber;
    
    // 將本地座標轉換為螢幕座標
    juce::Point<int> screenPos = localPointToGlobal(localPosition);
    
    SourceEditWindow::showModal("Add Source", info, 
        [this, localPosition](const SourceEditWindow::SourceInfo& sourceInfo)
        {
            // 創建新球，位置在右鍵點擊的位置
            int newBallId = sourceInfo.sourceNumber;
            auto logicPos = screenToLogic(localPosition);
            
            jyPad.addBall(newBallId, logicPos.x, logicPos.y);
            
            // 更新球的 source 資訊
            auto* ball = jyPad.getBall(newBallId);
            if (ball != nullptr)
            {
                ball->oscPrefix = sourceInfo.oscPrefix;
                ball->color = sourceInfo.color;
                ball->sourceName = sourceInfo.sourceName;
                ball->sourceNumber = sourceInfo.sourceNumber;
            }
            
            repaint();
        },
        screenPos);
}

void JYPadEditor::showEditSourceMenu(int ballId)
{
    auto* ball = jyPad.getBall(ballId);
    if (ball == nullptr)
        return;
    
    SourceEditWindow::SourceInfo info;
    info.oscPrefix = ball->oscPrefix;
    info.color = ball->color;
    info.sourceName = ball->sourceName;
    info.sourceNumber = ball->sourceNumber;
    
    // 獲取滑鼠位置
    juce::Point<int> mousePos = juce::Desktop::getInstance().getMousePosition();
    
    SourceEditWindow::showModal("Edit Source", info,
        [this, ballId](const SourceEditWindow::SourceInfo& sourceInfo)
        {
            auto* ball = jyPad.getBall(ballId);
            if (ball != nullptr)
            {
                ball->oscPrefix = sourceInfo.oscPrefix;
                ball->color = sourceInfo.color;
                ball->sourceName = sourceInfo.sourceName;
                ball->sourceNumber = sourceInfo.sourceNumber;
                repaint();
            }
        },
        mousePos);
}

void JYPadEditor::showDeleteConfirmMenu(int ballId)
{
    juce::AlertWindow::showOkCancelBox(
        juce::MessageBoxIconType::QuestionIcon,
        "Delete Source",
        "Are you sure you want to delete this source? This will also remove all related data from OSC DATA.",
        "Delete",
        "Cancel",
        this,
        juce::ModalCallbackFunction::create(
            [this, ballId](int result)
            {
                if (result == 1)  // OK
                {
                    // 從 OSC data 中刪除該 source 的所有資料
                    auto* ball = jyPad.getBall(ballId);
                    if (ball != nullptr)
                    {
                        audioProcessor.dataTable.removeRowsBySourceNumber(ball->sourceNumber);
                    }
                    
                    // 刪除球
                    jyPad.removeBall(ballId);
                    repaint();
                }
            }
        )
    );
}

