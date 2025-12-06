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
        ball->x = juce::jlimit(-1.0f, 1.0f, x);
        ball->y = juce::jlimit(-1.0f, 1.0f, y);

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
    events.emplace_back(ballId, midiTime, x, y, z);
    
    // 保持按時間排序（雖然通常新事件時間會更晚，但為了安全還是排序）
    std::sort(events.begin(), events.end());
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

void JYPad::tweenToNext(int ballId, double currentMidiTime, double bpm, 
                        int timeSignatureNumerator, int timeSignatureDenominator)
{
    // 檢查球是否存在
    auto* ball = findBall(ballId);
    if (ball == nullptr)
        return;
    
    auto it = recordedEvents.find(ballId);
    if (it == recordedEvents.end() || it->second.empty())
        return;
    
    auto& events = it->second;
    
    // 找到下一個事件（時間大於當前時間的第一個事件）
    const RecordedEvent* nextEvent = nullptr;
    double nextTime = -1.0;
    
    for (size_t i = 0; i < events.size(); ++i)
    {
        if (events[i].midiTime > currentMidiTime)
        {
            if (nextTime < 0.0 || events[i].midiTime < nextTime)
            {
                nextTime = events[i].midiTime;
                nextEvent = &events[i];
            }
        }
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
    
    // 分析現有錄製事件的時間間隔，以決定插值密度
    // 計算該球所有事件之間的平均間隔
    double averageInterval = 0.0;
    int intervalCount = 0;
    
    if (events.size() >= 2)
    {
        double totalInterval = 0.0;
        for (size_t i = 1; i < events.size(); ++i)
        {
            double interval = events[i].midiTime - events[i-1].midiTime;
            if (interval > 0.0)  // 只計算正間隔
            {
                totalInterval += interval;
                intervalCount++;
            }
        }
        
        if (intervalCount > 0)
        {
            averageInterval = totalInterval / static_cast<double>(intervalCount);
        }
    }
    
    // 如果沒有足夠的事件來計算平均間隔，使用一個合理的默認值
    // 默認值：約等於 20ms 的更新頻率（假設 BPM=120，一個四分音符=960 ticks）
    // 20ms 在 120 BPM 下約等於 960 * (20/1000) * (120/60) / 4 = 96 ticks
    if (averageInterval <= 0.0)
    {
        const double ticksPerQuarter = 960.0;
        // 假設更新頻率約為 20ms，在 120 BPM 下
        averageInterval = ticksPerQuarter * (20.0 / 1000.0) * (bpm / 60.0) / 4.0;
    }
    
    // 根據平均間隔計算需要多少個插值事件
    // 如果時間範圍是 timeRange，平均間隔是 averageInterval，則需要 timeRange / averageInterval 個事件
    int numSteps = static_cast<int>(std::round(timeRange / averageInterval));
    
    // 確保至少有一個步驟，但也不要太多（限制在合理範圍內，最多不超過時間範圍的 100 倍）
    numSteps = juce::jlimit(1, static_cast<int>(std::max(100.0, timeRange * 100.0)), numSteps);
    
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
    
    // 找到最接近當前時間的事件（小於等於當前時間的最大事件）
    // 使用二分查找優化（因為事件已按時間排序）
    const RecordedEvent* closestEvent = nullptr;
    double closestTime = -1.0;
    
    // 線性搜索（因為需要找到 <= midiTime 的最大值）
    for (const auto& event : events)
    {
        if (event.midiTime <= midiTime)
        {
            if (event.midiTime > closestTime)
            {
                closestTime = event.midiTime;
                closestEvent = &event;
            }
        }
        else
        {
            // 事件已按時間排序，如果當前事件時間 > midiTime，後面的也會更大
            break;
        }
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
    
    // 從後往前找，找到第一個時間 <= midiTime 的事件
    for (auto rit = events.rbegin(); rit != events.rend(); ++rit)
    {
        if (rit->midiTime <= midiTime)
        {
            return &(*rit);
        }
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

void JYPad::resetBallsToFirstEventOrCenter()
{
    for (auto& ball : balls)
    {
        const RecordedEvent* firstEvent = getFirstEvent(ball.id);
        if (firstEvent != nullptr)
        {
            // 有錄製數據，重置到第一個事件的位置
            ball.x = firstEvent->x;
            ball.y = firstEvent->y;
        }
        else
        {
            // 沒有錄製數據，重置到中心 (0, 0)
            ball.x = 0.0f;
            ball.y = 0.0f;
        }
    }
}

