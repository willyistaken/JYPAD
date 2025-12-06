#pragma once

#include <juce_core/juce_core.h>
#include <fstream>
#include <mutex>

//==============================================================================
/**
 * Debug Logger
 * 用於輸出調試日誌到文件
 */
class DebugLogger
{
public:
    static DebugLogger& getInstance()
    {
        static DebugLogger instance;
        return instance;
    }
    
    void log(const juce::String& message)
    {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (!logFile.is_open())
        {
            // 獲取用戶文檔目錄
            juce::File logDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                                .getChildFile("PlugDataCustomObject");
            logDir.createDirectory();
            
            juce::File logFilePath = logDir.getChildFile("debug.log");
            logFile.open(logFilePath.getFullPathName().toRawUTF8(), std::ios::app);
            
            if (logFile.is_open())
            {
                logFile << "\n=== Session Started: " << juce::Time::getCurrentTime().toString(true, true) << " ===\n";
                logFile.flush();
            }
            else
            {
                // 如果無法打開文件，至少輸出到控制台
                juce::Logger::writeToLog("WARNING: Cannot open log file: " + logFilePath.getFullPathName());
            }
        }
        
        if (logFile.is_open())
        {
            juce::String timestamp = juce::Time::getCurrentTime().toString(true, true);
            logFile << "[" << timestamp << "] " << message << "\n";
            logFile.flush();
        }
        
        // 同時輸出到控制台（如果可用）
        juce::Logger::writeToLog(message);
    }
    
    void logError(const juce::String& message)
    {
        log("ERROR: " + message);
    }
    
    void logWarning(const juce::String& message)
    {
        log("WARNING: " + message);
    }
    
    ~DebugLogger()
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (logFile.is_open())
        {
            logFile << "=== Session Ended ===\n\n";
            logFile.close();
        }
    }
    
private:
    DebugLogger() = default;
    DebugLogger(const DebugLogger&) = delete;
    DebugLogger& operator=(const DebugLogger&) = delete;
    
    std::ofstream logFile;
    std::mutex mutex;
};

// 便捷宏
// Release 版本：禁用所有 debug log
#ifdef JUCE_DEBUG
    #define DEBUG_LOG(msg) DebugLogger::getInstance().log(msg)
    #define DEBUG_LOG_ERROR(msg) DebugLogger::getInstance().logError(msg)
    #define DEBUG_LOG_WARNING(msg) DebugLogger::getInstance().logWarning(msg)
#else
    // Release 版本：不輸出任何 log
    #define DEBUG_LOG(msg) ((void)0)
    #define DEBUG_LOG_ERROR(msg) ((void)0)
    #define DEBUG_LOG_WARNING(msg) ((void)0)
#endif

