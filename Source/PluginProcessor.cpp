#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DebugLogger.h"

//==============================================================================
PlugDataCustomObjectAudioProcessor::PlugDataCustomObjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    DEBUG_LOG("PluginProcessor: Constructor started");
    
    try
    {
        DEBUG_LOG("PluginProcessor: Initializing JYPad");
        // JYPad 會在構造函數中自動初始化
        
        DEBUG_LOG("PluginProcessor: Initializing DataTable");
        // DataTable 會在構造函數中自動初始化
        
        DEBUG_LOG("PluginProcessor: Initializing OSC connection");
        // 初始化 OSC 連接
        updateOSCConnection();
        
        DEBUG_LOG("PluginProcessor: Constructor completed successfully");
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("PluginProcessor: Exception in constructor: " + juce::String(e.what()));
        throw;
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("PluginProcessor: Unknown exception in constructor");
        throw;
    }
}

PlugDataCustomObjectAudioProcessor::~PlugDataCustomObjectAudioProcessor()
{
}

//==============================================================================
const juce::String PlugDataCustomObjectAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PlugDataCustomObjectAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PlugDataCustomObjectAudioProcessor::producesMidi() const
{
   #if JucePlugin_WantsMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PlugDataCustomObjectAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PlugDataCustomObjectAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PlugDataCustomObjectAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PlugDataCustomObjectAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PlugDataCustomObjectAudioProcessor::setCurrentProgram ([[maybe_unused]] int index)
{
}

const juce::String PlugDataCustomObjectAudioProcessor::getProgramName ([[maybe_unused]] int index)
{
    return {};
}

void PlugDataCustomObjectAudioProcessor::changeProgramName ([[maybe_unused]] int index, [[maybe_unused]] const juce::String& newName)
{
}

//==============================================================================
void PlugDataCustomObjectAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // 初始化 JYPad
    jyPad.prepare(sampleRate, samplesPerBlock);
}

void PlugDataCustomObjectAudioProcessor::releaseResources()
{
    // 釋放資源
    jyPad.release();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PlugDataCustomObjectAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PlugDataCustomObjectAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // 清除未使用的輸出通道
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // 在 processBlock 中獲取時間碼資訊（只能在這裡調用 getPlayHead）
    {
        juce::ScopedLock lock(timeCodeLock);
        cachedTimeCodeInfo = TimeCodeInfo();  // 重置
        
        auto* playHead = getPlayHead();
        if (playHead != nullptr)
        {
            juce::AudioPlayHead::CurrentPositionInfo positionInfo;
            if (playHead->getCurrentPosition(positionInfo))
            {
                cachedTimeCodeInfo.bpm = positionInfo.bpm;
                cachedTimeCodeInfo.timeInSeconds = positionInfo.timeInSeconds;
                cachedTimeCodeInfo.ppqPosition = positionInfo.ppqPosition;
                cachedTimeCodeInfo.isPlaying = positionInfo.isPlaying;
                cachedTimeCodeInfo.timeSignatureNumerator = positionInfo.timeSigNumerator;
                cachedTimeCodeInfo.timeSignatureDenominator = positionInfo.timeSigDenominator;
                cachedTimeCodeInfo.ppqPositionOfLastBarStart = positionInfo.ppqPositionOfLastBarStart;
                cachedTimeCodeInfo.isValid = true;
            }
        }
    }

    // 處理音訊（JYPad 主要用於控制，但保留音訊處理能力）
    jyPad.processBlock(buffer, midiMessages);
}

//==============================================================================
bool PlugDataCustomObjectAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PlugDataCustomObjectAudioProcessor::createEditor()
{
    DEBUG_LOG("PluginProcessor: createEditor called - START");
    try
    {
        DEBUG_LOG("PluginProcessor: About to create PluginEditor instance");
        auto* editor = new PlugDataCustomObjectAudioProcessorEditor (*this);
        setEditor(editor);  // 設置 Editor 指針，用於記錄 OSC 訊息
        DEBUG_LOG("PluginProcessor: Editor instance created, returning");
        return editor;
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("PluginProcessor: Exception in createEditor: " + juce::String(e.what()));
        throw;
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("PluginProcessor: Unknown exception in createEditor");
        throw;
    }
}

//==============================================================================
void PlugDataCustomObjectAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // 儲存狀態
    juce::MemoryOutputStream mos(destData, true);
    
    // 保存 JYPad 狀態
    jyPad.saveState(mos);
    
    // 保存數據表格狀態
    dataTable.saveState(mos);
    
    // 保存 OSC 設置
    {
        juce::ScopedLock lock(oscSettingsLock);
        mos.writeString(oscSettings.ipAddress);
        mos.writeInt(oscSettings.port);
        mos.writeBool(oscSettings.enabled);
    }
    
    // 保存 zoom scale
    mos.writeFloat(zoomScale);
}

void PlugDataCustomObjectAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    DEBUG_LOG("PluginProcessor: setStateInformation called, size: " + juce::String(sizeInBytes));
    try
    {
        // 載入狀態
        juce::MemoryInputStream mis(data, static_cast<size_t>(sizeInBytes), false);
        
        DEBUG_LOG("PluginProcessor: Loading JYPad state");
        // 載入 JYPad 狀態
        jyPad.loadState(mis);
        DEBUG_LOG("PluginProcessor: JYPad state loaded");
        
        DEBUG_LOG("PluginProcessor: Loading DataTable state");
        // 載入數據表格狀態
        dataTable.loadState(mis);
        DEBUG_LOG("PluginProcessor: DataTable state loaded");
        
        // 載入 OSC 設置（如果存在）
        if (!mis.isExhausted())
        {
            DEBUG_LOG("PluginProcessor: Loading OSC settings");
            {
                juce::ScopedLock lock(oscSettingsLock);
                oscSettings.ipAddress = mis.readString();
                oscSettings.port = mis.readInt();
                oscSettings.enabled = mis.readBool();
            }
            updateOSCConnection();
            DEBUG_LOG("PluginProcessor: OSC settings loaded");
        }
        else
        {
            DEBUG_LOG("PluginProcessor: No OSC settings in state");
        }
        
        // 載入 zoom scale（如果存在）
        if (!mis.isExhausted())
        {
            DEBUG_LOG("PluginProcessor: Loading zoom scale");
            zoomScale = mis.readFloat();
            // 限制在有效範圍內
            zoomScale = juce::jlimit(0.1f, 10.0f, zoomScale);
            DEBUG_LOG("PluginProcessor: Zoom scale loaded: " + juce::String(zoomScale));
        }
        else
        {
            DEBUG_LOG("PluginProcessor: No zoom scale in state, using default");
        }
        
        DEBUG_LOG("PluginProcessor: setStateInformation completed");
    }
    catch (const std::exception& e)
    {
        DEBUG_LOG_ERROR("PluginProcessor: Exception in setStateInformation: " + juce::String(e.what()));
        // 重置為預設狀態（清除所有球並添加一個預設球）
        jyPad.clearBalls();
        jyPad.addBall(1, 0.0f, 0.0f);
        dataTable.clear();
    }
    catch (...)
    {
        DEBUG_LOG_ERROR("PluginProcessor: Unknown exception in setStateInformation");
        // 重置為預設狀態
        jyPad.clearBalls();
        jyPad.addBall(1, 0.0f, 0.0f);
        dataTable.clear();
    }
}

//==============================================================================
void PlugDataCustomObjectAudioProcessor::updateOSCConnection()
{
    oscSender.disconnect();
    
    OSCSettings settings;
    {
        juce::ScopedLock lock(oscSettingsLock);
        settings = oscSettings;
    }
    
    if (settings.enabled)
    {
        if (oscSender.connect(settings.ipAddress, settings.port))
        {
            DBG("OSC connected to " + settings.ipAddress + ":" + juce::String(settings.port));
        }
        else
        {
            DBG("Failed to connect OSC to " + settings.ipAddress + ":" + juce::String(settings.port));
        }
    }
}

void PlugDataCustomObjectAudioProcessor::sendOSCMessage(int ballId, float x, float y, [[maybe_unused]] float z)
{
    bool enabled;
    {
        juce::ScopedLock lock(oscSettingsLock);
        enabled = oscSettings.enabled;
    }
    
    if (!enabled)
        return;
    
    // 獲取球的 oscPrefix
    Ball* ball = jyPad.getBall(ballId);
    if (ball == nullptr)
        return;
    
    // OSC 發送格式：{osc_prefix}/xy x y（暫時不發送 z 值）
    // 例如：如果 oscPrefix = "/track/1"，則地址為 "/track/1/xy"，參數為 x, y
    juce::String address = ball->oscPrefix + "/xy";
    juce::OSCMessage message(address, x, y); // 只發送 x, y
    
    // 記錄 OSC 訊息
    if (oscMessageEditor != nullptr)
    {
        juce::String logMsg = address + " " + juce::String(x, 2) + " " + juce::String(y, 2);
        oscMessageEditor->logOSCMessage(logMsg);
    }
    
    if (!oscSender.send(message))
    {
        // 如果發送失敗，嘗試重新連接
        updateOSCConnection();
        oscSender.send(message);
    }
}

void PlugDataCustomObjectAudioProcessor::sendMuteSoloOSCMessage(int ballId, bool isMute, bool isSolo)
{
    bool enabled;
    {
        juce::ScopedLock lock(oscSettingsLock);
        enabled = oscSettings.enabled;
    }
    
    if (!enabled)
        return;
    
    // 獲取球的信息
    Ball* ball = jyPad.getBall(ballId);
    if (ball == nullptr)
        return;
    
    // 發送 mute 訊息：{osc_prefix}/n/mute 1 或 0
    // 其中 n 是 source number
    // 例如：如果 oscPrefix = "/track/1"，sourceNumber = 1，則地址為 "/track/1/1/mute"
    // 但根據用戶需求，應該是 {osc_prefix}/n/mute，其中 osc_prefix 可能已經是 "/track"，n 是 source number
    // 為了保持一致性，我們假設 oscPrefix 是基礎前綴（如 "/track"），然後加上 source number
    // 但如果 oscPrefix 已經包含 source number（如 "/track/1"），我們需要提取基礎前綴
    // 暫時假設 oscPrefix 格式為 "/track/n"，我們需要提取 "/track" 部分
    juce::String basePrefix = ball->oscPrefix;
    // 如果 oscPrefix 以 "/track/" 開頭，提取基礎前綴
    if (basePrefix.startsWith("/track/"))
    {
        // 提取 "/track" 部分
        int lastSlash = basePrefix.lastIndexOfChar('/');
        if (lastSlash > 0)
        {
            basePrefix = basePrefix.substring(0, lastSlash);
        }
    }
    
    juce::String muteAddress = basePrefix + "/" + juce::String(ball->sourceNumber) + "/mute";
    juce::OSCMessage muteMessage(muteAddress, isMute ? 1 : 0);
    
    // 記錄 OSC 訊息
    if (oscMessageEditor != nullptr)
    {
        juce::String logMsg = muteAddress + " " + juce::String(isMute ? 1 : 0);
        oscMessageEditor->logOSCMessage(logMsg);
    }
    
    if (!oscSender.send(muteMessage))
    {
        updateOSCConnection();
        oscSender.send(muteMessage);
    }
    
    // 發送 solo 訊息：{osc_prefix}/n/solo 1 或 0
    juce::String soloAddress = basePrefix + "/" + juce::String(ball->sourceNumber) + "/solo";
    juce::OSCMessage soloMessage(soloAddress, isSolo ? 1 : 0);
    
    // 記錄 OSC 訊息
    if (oscMessageEditor != nullptr)
    {
        juce::String logMsg = soloAddress + " " + juce::String(isSolo ? 1 : 0);
        oscMessageEditor->logOSCMessage(logMsg);
    }
    
    if (!oscSender.send(soloMessage))
    {
        updateOSCConnection();
        oscSender.send(soloMessage);
    }
}

//==============================================================================
PlugDataCustomObjectAudioProcessor::TimeCodeInfo PlugDataCustomObjectAudioProcessor::getTimeCodeInfo() const
{
    // 從緩存中讀取時間碼資訊（線程安全）
    juce::ScopedLock lock(timeCodeLock);
    return cachedTimeCodeInfo;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PlugDataCustomObjectAudioProcessor();
}
