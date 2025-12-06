#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <functional>
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>

//==============================================================================
/**
 * 錄製的事件
 * 記錄球的位置變化
 */
struct RecordedEvent
{
    juce::Uuid ballUid;  // 使用 UID 而不是 ballId（系統產生的唯一識別碼）
    int ballId;  // 保留用於向後兼容和顯示
    double midiTime;  // PPQ position (MIDI time)
    float x;
    float y;
    float z;  // 暫時為 0，未來擴展用
    
    RecordedEvent(const juce::Uuid& uid, int id, double time, float xPos, float yPos, float zPos = 0.0f)
        : ballUid(uid), ballId(id), midiTime(time), x(xPos), y(yPos), z(zPos) {}
    
    // 用於排序（按時間排序）
    bool operator<(const RecordedEvent& other) const
    {
        return midiTime < other.midiTime;
    }
};

//==============================================================================
/**
 * JYPad 物件
 * 2D 平面控制器，可以顯示和控制多個圓球
 * 中心點為 (0, 0)，座標範圍為 -1.0 到 1.0
 */
struct Ball
{
    juce::Uuid uid;  // 系統產生的唯一識別碼，與 source number 無關
    int id;  // 保留用於向後兼容，但主要使用 uid
    float x;  // 範圍: -1.0 到 1.0
    float y;  // 範圍: -1.0 到 1.0
    
    // Source 資訊
    juce::String oscPrefix = "/track/1";  // 預設 prefix 為 /track/n
    juce::Colour color = juce::Colour(0xff4a90e2);  // 預設藍色
    juce::String sourceName = "";
    int sourceNumber = 1;  // 自動編號
    
    // Mute 和 Solo 狀態
    bool isMuted = false;
    bool isSoloed = false;
    
    // Recording 狀態
    bool isRecording = false;
    
    Ball(int ballId, float xPos = 0.0f, float yPos = 0.0f)
        : uid(juce::Uuid()), id(ballId), x(xPos), y(yPos) {}
    
    Ball(int ballId, float xPos, float yPos, const juce::String& prefix, 
         const juce::Colour& col, const juce::String& name, int number)
        : uid(juce::Uuid()), id(ballId), x(xPos), y(yPos), oscPrefix(prefix), color(col), 
          sourceName(name), sourceNumber(number) {}
};

class JYPad
{
public:
    JYPad();
    ~JYPad();

    // 準備處理
    void prepare(double sampleRate, int samplesPerBlock);

    // 釋放資源
    void release();

    // 處理音訊區塊（如果需要音訊處理）
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);

    // 球體管理
    void addBall(int ballId, float x = 0.0f, float y = 0.0f);
    void removeBall(int ballId);
    void setBallPosition(int ballId, float x, float y);
    Ball* getBall(int ballId);
    const std::vector<Ball>& getAllBalls() const { return balls; }
    std::vector<Ball>& getAllBalls() { return balls; }  // 非 const 版本，用於重置
    int getNumBalls() const { return static_cast<int>(balls.size()); }
    void clearBalls() { balls.clear(); }  // 清除所有球

    // 座標轉換（UI 座標 <-> 邏輯座標）
    // UI 座標：0.0-1.0，邏輯座標：-1.0 到 1.0（中心為 0,0）
    float uiToLogicX(float uiX) const { return (uiX * 2.0f) - 1.0f; }
    float uiToLogicY(float uiY) const { return 1.0f - (uiY * 2.0f); }  // 翻轉 Y 軸
    float logicToUiX(float logicX) const { return (logicX + 1.0f) * 0.5f; }
    float logicToUiY(float logicY) const { return (1.0f - logicY) * 0.5f; }

    // 輸出格式：球編號 x y（例如：1 0.3 0.5）
    juce::String getBallOutputString(int ballId) const;

    // 回調函數類型：當球移動時調用
    std::function<void(int ballId, float x, float y)> onBallMoved;

    // MIDI 錄製功能
    // 記錄球的位置變化（當球處於 recording 狀態且被拖動時）
    void recordEvent(int ballId, double midiTime, float x, float y, float z = 0.0f);
    
    // 清除指定球的錄製數據
    void clearRecordedEvents(int ballId);
    
    // 清除所有錄製數據
    void clearAllRecordedEvents();
    
    // 在指定時間點插入當前位置的事件
    void insertEventAtTime(int ballId, double midiTime, float x, float y, float z = 0.0f);
    
    // 在當前事件和下一個事件之間生成插值事件（Tween）
    void tweenToNext(int ballId, double currentMidiTime, int numSteps);
    
    // 根據 MIDI time 回放錄製的事件（返回需要更新的球位置，如果沒有變化則返回 nullptr）
    // 只在位置改變時返回，不動時返回 nullptr 以節省資源
    const RecordedEvent* getEventAtTime(int ballId, double midiTime) const;
    
    // 獲取指定球的第一個錄製事件（用於重置到初始位置）
    const RecordedEvent* getFirstEvent(int ballId) const;
    
    // 獲取指定時間點之前最後一個錄製事件（用於非播放狀態下的位置顯示）
    const RecordedEvent* getLastEventBeforeTime(int ballId, double midiTime) const;
    
    // 獲取指定球的錄製事件數量
    int getRecordedEventCount(int ballId) const;

    // 狀態儲存/載入
    void saveState(juce::MemoryOutputStream& stream);
    void loadState(juce::MemoryInputStream& stream);
    
    // 重置所有球到第一個事件或中心
    void resetBallsToFirstEventOrCenter();

private:
    std::vector<Ball> balls;
    double currentSampleRate = 44100.0;
    int currentBlockSize = 512;
    
    // 錄製的事件數據：每個球 UID 對應一個事件序列（按時間排序）
    std::map<juce::Uuid, std::vector<RecordedEvent>> recordedEvents;
    
    // UID 到 ballId 的映射（用於向後兼容）
    std::map<juce::Uuid, int> uidToIdMap;

    Ball* findBall(int ballId);
    const Ball* findBall(int ballId) const;
    Ball* findBallByUid(const juce::Uuid& uid);
    const Ball* findBallByUid(const juce::Uuid& uid) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JYPad)
};

