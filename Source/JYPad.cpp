#include "JYPad.h"
#include "DebugLogger.h"
#include <algorithm>

//==============================================================================
JYPad::JYPad()
{
    DEBUG_LOG("JYPad: Constructor started");
    try
    {
        // 初始化錄製事件 map（確保是空的）
        recordedEvents.clear();
        
        // 預設添加一個球
        DEBUG_LOG("JYPad: Adding default ball");
        addBall(1, 0.0f, 0.0f);
        DEBUG_LOG("JYPad: Constructor completed");
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("JYPad: Exception in constructor: " + juce::String(e.what()));
        recordedEvents.clear();  // 確保清理
        throw;
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("JYPad: Unknown exception in constructor");
        recordedEvents.clear();  // 確保清理
        throw;
    }
}

JYPad::~JYPad()
{
}

//==============================================================================
void JYPad::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
}

void JYPad::release()
{
}

//==============================================================================
void JYPad::processBlock([[maybe_unused]] juce::AudioBuffer<float>& buffer, [[maybe_unused]] juce::MidiBuffer& midiMessages)
{
    // JYPad 主要用於控制，不需要音訊處理
    // 但保留此方法以備將來擴展
}

//==============================================================================
void JYPad::addBall(int ballId, float x, float y)
{
    // 檢查是否已存在
    if (findBall(ballId) != nullptr)
        return;

    // 限制座標範圍
    x = juce::jlimit(-1.0f, 1.0f, x);
    y = juce::jlimit(-1.0f, 1.0f, y);

    balls.emplace_back(ballId, x, y);
}

void JYPad::removeBall(int ballId)
{
    // 刪除球
    balls.erase(
        std::remove_if(balls.begin(), balls.end(),
            [ballId](const Ball& ball) { return ball.id == ballId; }),
        balls.end()
    );
    
    // 同時刪除該球的錄製事件數據
    clearRecordedEvents(ballId);
}

void JYPad::setBallPosition(int ballId, float x, float y)
{
    Ball* ball = findBall(ballId);
    if (ball != nullptr)
    {
        // 限制座標範圍
        // Limit coordinate range
        float newX = juce::jlimit(-1.0f, 1.0f, x);
        float newY = juce::jlimit(-1.0f, 1.0f, y);

        // Optimization: Check if position actually changed (epsilon check)
        if (std::abs(ball->x - newX) < 1e-5f && std::abs(ball->y - newY) < 1e-5f)
            return;

        ball->x = newX;
        ball->y = newY;

        // 觸發回調
        if (onBallMoved)
            onBallMoved(ballId, ball->x, ball->y);
    }
}

Ball* JYPad::getBall(int ballId)
{
    return findBall(ballId);
}

Ball* JYPad::findBall(int ballId)
{
    auto it = std::find_if(balls.begin(), balls.end(),
        [ballId](const Ball& ball) { return ball.id == ballId; });
    return (it != balls.end()) ? &(*it) : nullptr;
}

const Ball* JYPad::findBall(int ballId) const
{
    auto it = std::find_if(balls.begin(), balls.end(),
        [ballId](const Ball& ball) { return ball.id == ballId; });
    return (it != balls.end()) ? &(*it) : nullptr;
}

//==============================================================================
juce::String JYPad::getBallOutputString(int ballId) const
{
    const Ball* ball = findBall(ballId);
    if (ball != nullptr)
    {
        // 輸出格式：球編號 x y（例如：1 0.3 0.5）
        return juce::String(ball->id) + " " + 
               juce::String(ball->x, 3) + " " + 
               juce::String(ball->y, 3);
    }
    return "";
}

//==============================================================================
void JYPad::saveState(juce::MemoryOutputStream& stream)
{
    stream.writeInt(static_cast<int>(balls.size()));
    for (const auto& ball : balls)
    {
        stream.writeInt(ball.id);
        stream.writeFloat(ball.x);
        stream.writeFloat(ball.y);
        stream.writeString(ball.oscPrefix);
        stream.writeInt(ball.color.getARGB());
        stream.writeString(ball.sourceName);
        stream.writeInt(ball.sourceNumber);
        stream.writeBool(ball.isMuted);
        stream.writeBool(ball.isSoloed);
        stream.writeBool(ball.isRecording);
    }
    
    // 保存錄製的事件數據
    // 先寫入一個標記值，用於檢測是否有錄製事件數據（0x5245434D = "RECM" 的 ASCII）
    // 這樣在載入時可以檢測是否是錄製事件數據
    const int RECORDED_EVENTS_MARKER = 0x5245434D;  // "RECM"
    stream.writeInt(RECORDED_EVENTS_MARKER);
    stream.writeInt(static_cast<int>(recordedEvents.size()));
    for (const auto& pair : recordedEvents)
    {
        int ballId = pair.first;
        const auto& events = pair.second;
        stream.writeInt(ballId);
        stream.writeInt(static_cast<int>(events.size()));
        for (const auto& event : events)
        {
            stream.writeInt(event.ballId);
            stream.writeDouble(event.midiTime);
            stream.writeFloat(event.x);
            stream.writeFloat(event.y);
            stream.writeFloat(event.z);
        }
    }
}

void JYPad::loadState(juce::MemoryInputStream& stream)
{
    DEBUG_LOG("JYPad: loadState started");
    try
    {
        balls.clear();
        
        // 檢查流是否已經耗盡
        if (stream.isExhausted())
        {
            DEBUG_LOG("JYPad: Stream is exhausted, no data to load");
            return;
        }
        
        int numBalls = stream.readInt();
        DEBUG_LOG("JYPad: Loading " + juce::String(numBalls) + " balls");
        
        // 檢查 numBalls 是否合理
        if (numBalls < 0 || numBalls > 1000)
        {
            DEBUG_LOG_ERROR("JYPad: Invalid number of balls: " + juce::String(numBalls) + ", resetting");
            balls.clear();
            addBall(1, 0.0f, 0.0f);  // 添加預設球
            return;
        }
        
        for (int i = 0; i < numBalls; ++i)
        {
            // 檢查流是否還有數據
            if (stream.isExhausted())
            {
                DEBUG_LOG_ERROR("JYPad: Stream exhausted at ball " + juce::String(i) + ", stopping");
                break;
            }
            
            int id = stream.readInt();
            DEBUG_LOG("JYPad: Reading ball " + juce::String(i) + ", id=" + juce::String(id));
            
            // 檢查 ID 是否合理
            if (id < 0 || id > 10000)
            {
                DEBUG_LOG_ERROR("JYPad: Invalid ball ID: " + juce::String(id) + ", skipping");
                // 嘗試跳過這個球的數據，但這很危險，最好重置
                break;
            }
            
            if (stream.isExhausted())
            {
                DEBUG_LOG_ERROR("JYPad: Stream exhausted after reading id, stopping");
                break;
            }
            
            float x = stream.readFloat();
            
            if (stream.isExhausted())
            {
                DEBUG_LOG_ERROR("JYPad: Stream exhausted after reading x, stopping");
                break;
            }
            
            float y = stream.readFloat();
            
            // 嘗試讀取新欄位（向後兼容）
            if (!stream.isExhausted())
            {
                try
                {
                    juce::String prefix = stream.readString();
                    if (stream.isExhausted()) throw std::runtime_error("Stream exhausted after prefix");
                    
                    int colorARGB = stream.readInt();
                    if (stream.isExhausted()) throw std::runtime_error("Stream exhausted after color");
                    
                    juce::String name = stream.readString();
                    if (stream.isExhausted()) throw std::runtime_error("Stream exhausted after name");
                    
                    int number = stream.readInt();
                    if (stream.isExhausted()) throw std::runtime_error("Stream exhausted after number");
                    
                    Ball newBall(id, x, y, prefix, juce::Colour(colorARGB), name, number);
                    
                    // 嘗試讀取 mute 和 solo 狀態（向後兼容）
                    if (!stream.isExhausted())
                    {
                        try
                        {
                            newBall.isMuted = stream.readBool();
                            if (stream.isExhausted())
                            {
                                newBall.isSoloed = false;
                                DEBUG_LOG("JYPad: Stream exhausted after isMuted, using default for isSoloed");
                            }
                            else
                            {
                                newBall.isSoloed = stream.readBool();
                                
                                // 嘗試讀取 recording 狀態（向後兼容）
                                if (!stream.isExhausted())
                                {
                                    try
                                    {
                                        newBall.isRecording = stream.readBool();
                                    }
                                    catch (...)
                                    {
                                        newBall.isRecording = false;
                                    }
                                }
                                else
                                {
                                    newBall.isRecording = false;
                                }
                            }
                            DEBUG_LOG("JYPad: Loaded ball " + juce::String(id) + " with mute=" + 
                                     juce::String(newBall.isMuted ? 1 : 0) + " solo=" + 
                                     juce::String(newBall.isSoloed ? 1 : 0) + " recording=" +
                                     juce::String(newBall.isRecording ? 1 : 0));
                        }
                        catch (...)
                        {
                            // 如果讀取失敗，使用預設值
                            DEBUG_LOG("JYPad: Failed to read mute/solo/recording for ball " + juce::String(id) + ", using defaults");
                            newBall.isMuted = false;
                            newBall.isSoloed = false;
                            newBall.isRecording = false;
                        }
                    }
                    else
                    {
                        DEBUG_LOG("JYPad: Stream exhausted, using default mute/solo/recording for ball " + juce::String(id));
                        newBall.isMuted = false;
                        newBall.isSoloed = false;
                        newBall.isRecording = false;
                    }
                    
                    balls.push_back(newBall);
                }
                catch (const std::exception& e)
                {
                    DEBUG_LOG_ERROR("JYPad: Exception reading ball " + juce::String(id) + ": " + juce::String(e.what()));
                    // 如果讀取失敗，使用舊格式
                    addBall(id, x, y);
                }
            }
            else
            {
                // 舊格式，使用預設值
                DEBUG_LOG("JYPad: Using old format for ball " + juce::String(id));
                addBall(id, x, y);
            }
        }
        DEBUG_LOG("JYPad: loadState completed, total balls: " + juce::String(static_cast<int>(balls.size())));
        
        // 載入錄製的事件數據（如果存在）
        // 先清除舊的錄製事件，確保乾淨的狀態
        recordedEvents.clear();
        
        // 檢查是否還有數據（可能是錄製事件數據，也可能是其他數據）
        // 為了安全，我們先嘗試讀取一個標記值
        if (!stream.isExhausted())
        {
            try
            {
                // 讀取標記值，用於檢測是否有錄製事件數據
                const int RECORDED_EVENTS_MARKER = 0x5245434D;  // "RECM"
                int marker = stream.readInt();
                
                // 如果標記不匹配，說明沒有錄製事件數據（可能是舊格式或其他數據）
                if (marker != RECORDED_EVENTS_MARKER)
                {
                    // 標記不匹配，可能是舊格式或其他數據，跳過錄製事件的載入
                    DEBUG_LOG("JYPad: No recorded events marker found (got 0x" + 
                             juce::String::toHexString(marker) + "), likely old format, skipping recorded events");
                    // 恢復 stream 位置，讓後續的 DataTable 正常處理
                    int64_t currentPos = stream.getPosition();
                    stream.setPosition(currentPos - 4);  // 回退 4 字節（一個 int）
                    return;  // 提前返回，不載入錄製事件
                }
                
                // 標記匹配，繼續讀取錄製事件數量
                int numBallsWithEvents = stream.readInt();
                
                // 驗證值的合理性（防止讀取到錯誤的數據）
                if (numBallsWithEvents < 0 || numBallsWithEvents > 1000)
                {
                    DEBUG_LOG_ERROR("JYPad: Invalid numBallsWithEvents: " + juce::String(numBallsWithEvents));
                    return;  // 提前返回，不載入錄製事件
                }
                
                for (int i = 0; i < numBallsWithEvents; ++i)
                {
                    if (stream.isExhausted())
                        break;
                    
                    int ballId = stream.readInt();
                    if (stream.isExhausted())
                        break;
                    
                    int numEvents = stream.readInt();
                    
                    // 驗證 numEvents 的合理性
                    if (numEvents < 0 || numEvents > 100000)  // 最多 10 萬個事件
                    {
                        DEBUG_LOG_ERROR("JYPad: Invalid numEvents: " + juce::String(numEvents) + " for ball " + juce::String(ballId));
                        break;  // 跳過這個球的事件
                    }
                    
                    auto& events = recordedEvents[ballId];
                    events.clear();
                    events.reserve(numEvents);
                    
                    for (int j = 0; j < numEvents; ++j)
                    {
                        if (stream.isExhausted())
                        {
                            DEBUG_LOG_ERROR("JYPad: Stream exhausted while reading event " + juce::String(j) + " for ball " + juce::String(ballId));
                            break;
                        }
                        
                        int eventBallId = stream.readInt();
                        if (stream.isExhausted())
                            break;
                        
                        double midiTime = stream.readDouble();
                        if (stream.isExhausted())
                            break;
                        
                        float x = stream.readFloat();
                        if (stream.isExhausted())
                            break;
                        
                        float y = stream.readFloat();
                        if (stream.isExhausted())
                            break;
                        
                        float z = stream.readFloat();
                        
                        // 驗證數據的合理性
                        if (std::isfinite(midiTime) && std::isfinite(x) && std::isfinite(y) && std::isfinite(z))
                        {
                            events.emplace_back(eventBallId, midiTime, x, y, z);
                        }
                        else
                        {
                            DEBUG_LOG_ERROR("JYPad: Invalid event data (NaN/Inf) for ball " + juce::String(ballId));
                            // 跳過這個無效事件
                        }
                    }
                    
                    // 確保事件按時間排序
                    if (!events.empty())
                    {
                        std::sort(events.begin(), events.end());
                    }
                }
                DEBUG_LOG("JYPad: Loaded recorded events for " + juce::String(numBallsWithEvents) + " balls");
            }
            catch (const std::exception& e)
            {
                DEBUG_LOG_ERROR("JYPad: Exception loading recorded events: " + juce::String(e.what()));
                recordedEvents.clear();
            }
            catch (...)
            {
                DEBUG_LOG("JYPad: Failed to load recorded events (unknown exception), using defaults");
                recordedEvents.clear();
            }
        }
        else
        {
            DEBUG_LOG("JYPad: No recorded events data in stream (stream exhausted)");
        }
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("JYPad: Exception in loadState: " + juce::String(e.what()));
        // 重置為預設狀態
        balls.clear();
        recordedEvents.clear();
        addBall(1, 0.0f, 0.0f);
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("JYPad: Unknown exception in loadState");
        // 重置為預設狀態
        balls.clear();
        recordedEvents.clear();
        addBall(1, 0.0f, 0.0f);
    }
}

//==============================================================================
void JYPad::recordEvent(int ballId, double midiTime, float x, float y, float z)
{
    // 檢查球是否存在
    if (findBall(ballId) == nullptr)
        return;
    
    // 添加事件到對應球的錄製序列
    auto& events = recordedEvents[ballId];
    
    // 檢查是否需要排序：只有當新事件的時間小於等於最後一個事件的時間時才需要排序
    // 大多數情況下，錄製是按時間順序進行的，所以不需要排序
    bool needsSort = false;
    if (!events.empty())
    {
        if (midiTime < events.back().midiTime)
        {
            needsSort = true;
        }
    }
    
    // 執行插入（只插入一次！）
    events.emplace_back(ballId, midiTime, x, y, z);
    
    // 只有在需要時才排序
    if (needsSort)
    {
        std::sort(events.begin(), events.end());
    }
}

void JYPad::clearRecordedEvents(int ballId)
{
    recordedEvents.erase(ballId);
}

void JYPad::clearAllRecordedEvents()
{
    recordedEvents.clear();
}

void JYPad::insertEventAtTime(int ballId, double midiTime, float x, float y, float z)
{
    // 檢查球是否存在
    if (findBall(ballId) == nullptr)
        return;
    
    // 添加事件到對應球的錄製序列
    auto& events = recordedEvents[ballId];
    events.emplace_back(ballId, midiTime, x, y, z);
    
    // 保持按時間排序
    std::sort(events.begin(), events.end());
}

void JYPad::tweenToNext(int ballId, double currentMidiTime, int numSteps)
{
    // 檢查球是否存在
    auto* ball = findBall(ballId);
    if (ball == nullptr)
        return;
    
    auto it = recordedEvents.find(ballId);
    if (it == recordedEvents.end() || it->second.empty())
        return;
    
    auto& events = it->second;
    
    // 優化：使用二分查找找到第一個大於 currentMidiTime 的元素
    // std::upper_bound 正好返回第一個大於 value 的元素 iterator
    auto itEvent = std::upper_bound(events.begin(), events.end(), currentMidiTime,
        [](double time, const RecordedEvent& event) {
            return time < event.midiTime;
        });
        
    const RecordedEvent* nextEvent = nullptr;
    double nextTime = -1.0;
    
    if (itEvent != events.end())
    {
        nextEvent = &(*itEvent);
        nextTime = nextEvent->midiTime;
    }
    
    // 如果沒有下一個事件，無法進行插值
    if (nextEvent == nullptr)
        return;
    
    // 使用當前球的位置作為起始點
    float startX = ball->x;
    float startY = ball->y;
    float startZ = 0.0f;  // z 軸暫時為 0
    
    // 計算插值
    double timeRange = nextTime - currentMidiTime;
    if (timeRange <= 0.0)
        return;
    
    // 生成插值事件（從當前位置到下一個事件位置）
    for (int i = 1; i < numSteps; ++i)
    {
        double t = static_cast<double>(i) / static_cast<double>(numSteps);
        double interpolatedTime = currentMidiTime + timeRange * t;
        
        // 線性插值位置（從當前球位置到下一個事件位置）
        float x = startX + (nextEvent->x - startX) * static_cast<float>(t);
        float y = startY + (nextEvent->y - startY) * static_cast<float>(t);
        float z = startZ + (nextEvent->z - startZ) * static_cast<float>(t);
        
        events.emplace_back(ballId, interpolatedTime, x, y, z);
    }
    
    // 重新排序
    std::sort(events.begin(), events.end());
}

const RecordedEvent* JYPad::getEventAtTime(int ballId, double midiTime) const
{
    auto it = recordedEvents.find(ballId);
    if (it == recordedEvents.end() || it->second.empty())
        return nullptr;
    
    const auto& events = it->second;
    
    // 優化：使用二分查找找到第一個大於 midiTime 的元素
    // std::upper_bound 返回第一個大於 value 的元素
    // 我們想要找的是 <= midiTime 的最後一個元素，所以應該是 upper_bound 的前一個
    auto upper = std::upper_bound(events.begin(), events.end(), midiTime,
        [](double time, const RecordedEvent& event) {
            return time < event.midiTime;
        });
    
    const RecordedEvent* closestEvent = nullptr;
    
    // 如果 upper 是 begin，表示所有元素都大於 midiTime，沒有符合條件的
    if (upper != events.begin())
    {
        // upper 的前一個元素就是 <= midiTime 的最後一個元素（也是最大的那個）
        closestEvent = &(*(upper - 1));
    }
    
    // 如果找到了事件，檢查位置是否與當前球的位置不同
    if (closestEvent != nullptr)
    {
        const Ball* ball = findBall(ballId);
        if (ball != nullptr)
        {
            // 只在位置改變時返回事件（節省資源）
            const float epsilon = 0.0001f;  // 浮點數比較的容差
            if (std::abs(ball->x - closestEvent->x) > epsilon ||
                std::abs(ball->y - closestEvent->y) > epsilon)
            {
                return closestEvent;
            }
        }
        else
        {
            // 如果球不存在，仍然返回事件（可能球被刪除了但事件還在）
            return closestEvent;
        }
    }
    
    return nullptr;  // 沒有找到合適的事件，返回 nullptr
}

const RecordedEvent* JYPad::getFirstEvent(int ballId) const
{
    auto it = recordedEvents.find(ballId);
    if (it == recordedEvents.end() || it->second.empty())
        return nullptr;
    
    // 返回第一個事件（事件已按時間排序）
    return &(it->second.front());
}

const RecordedEvent* JYPad::getLastEventBeforeTime(int ballId, double midiTime) const
{
    auto it = recordedEvents.find(ballId);
    if (it == recordedEvents.end() || it->second.empty())
        return nullptr;
    
    const auto& events = it->second;
    
    // 優化：使用二分查找代替線性搜索
    // 我們找 <= midiTime 的最後一個元素
    auto upper = std::upper_bound(events.begin(), events.end(), midiTime,
        [](double time, const RecordedEvent& event) {
            return time < event.midiTime;
        });
        
    if (upper != events.begin())
    {
        return &(*(upper - 1));
    }
    
    // 如果所有事件的時間都大於 midiTime，返回 nullptr
    return nullptr;
}

int JYPad::getRecordedEventCount(int ballId) const
{
    auto it = recordedEvents.find(ballId);
    if (it == recordedEvents.end())
        return 0;
    return static_cast<int>(it->second.size());
}
