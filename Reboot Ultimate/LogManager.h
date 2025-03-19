#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <sstream>
#include <Windows.h>
#include <iostream>
#include <format>

enum class LogLevel 
{
    Info,
    Warning,
    Error,
    Debug
};

struct LogEntry 
{
    std::string message;
    LogLevel level;
};

class LogManager 
{
public:
    static void Init();
    static void Log(const std::string& msg, LogLevel level = LogLevel::Info);
    static const std::vector<LogEntry>& GetLogs();
    static void ClearLogs();
    static std::string GetLogDir();

    static std::vector<LogEntry> logs;
    static std::string logDir;
    static std::string currentLogFile;

    static void EnsureLogDir();
    static void MaintainLogLimit();

    static bool initialized;
    static void EnsureInitialized()
    {
        if (!initialized)
        {
            Init();
            initialized = true;
        }
    }
};

inline bool LogManager::initialized = false;

#define LOG_HELPER(level, loggerName, msg, ...)                \
    do {                                                       \
        LogManager::EnsureInitialized();                       \
        std::string formattedMsg = std::format(std::string("[{}] ") + msg, #loggerName __VA_OPT__(, ) __VA_ARGS__); \
        LogManager::Log(formattedMsg, level);                  \
    } while (0)

#define LOG_DEBUG(loggerName, ...)  LogManager::Log(std::format("[{}] {}", #loggerName, std::format(__VA_ARGS__)), LogLevel::Debug)
#define LOG_INFO(loggerName, ...)   LogManager::Log(std::format("[{}] {}", #loggerName, std::format(__VA_ARGS__)), LogLevel::Info)
#define LOG_WARN(loggerName, ...)   LogManager::Log(std::format("[{}] {}", #loggerName, std::format(__VA_ARGS__)), LogLevel::Warning)
#define LOG_ERROR(loggerName, ...)  LogManager::Log(std::format("[{}] {}", #loggerName, std::format(__VA_ARGS__)), LogLevel::Error)
#define LOG_FATAL(loggerName, ...)  LogManager::Log(std::format("[{}] {}", #loggerName, std::format(__VA_ARGS__)), LogLevel::Error)