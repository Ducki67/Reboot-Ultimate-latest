#include "LogManager.h"

std::vector<LogEntry> LogManager::logs;
std::string LogManager::logDir;
std::string LogManager::currentLogFile;

void LogManager::Init() 
{
    // FreeConsole();
    // AttachConsole(ATTACH_PARENT_PROCESS);

    SetConsoleTitleA("Reboot Ultimate");

    char* userPath = nullptr;
    size_t len;
    if (_dupenv_s(&userPath, &len, "USERPROFILE") == 0 && userPath != nullptr)
    {
        logDir = std::string(userPath) + "\\AppData\\Local\\Reboot Ultimate";
        free(userPath);
    }
    else
    {
        logDir = ".\\Logs";
    }

    EnsureLogDir();

    int logNumber = 1;

    for (int i = 1; i <= 5; ++i)
    {
        std::string logPath = logDir + "\\UltimateLog" + std::to_string(i) + ".log";

        if (!std::filesystem::exists(logPath))
        {
            logNumber = i;
            break;
        }
    }

    currentLogFile = logDir + "\\UltimateLog" + std::to_string(logNumber) + ".log";

    MaintainLogLimit();
}

void LogManager::Log(const std::string& msg, LogLevel level)
{
    EnsureInitialized();

    logs.push_back({ msg, level });

    EnsureLogDir();

    std::ofstream out(currentLogFile, std::ios::app);
    if (out.is_open())
    {
        out << msg << std::endl;
        out.close();
    }
}

const std::vector<LogEntry>& LogManager::GetLogs() 
{
    return logs;
}

void LogManager::EnsureLogDir() 
{
    std::filesystem::create_directories(logDir);
}

void LogManager::ClearLogs() 
{
    logs.clear();
}

std::string LogManager::GetLogDir() 
{
    return logDir;
}

void LogManager::MaintainLogLimit() 
{
    std::vector<std::filesystem::directory_entry> files;
    for (auto& entry : std::filesystem::directory_iterator(logDir)) 
    {
        if (entry.path().extension() == ".log")
            files.push_back(entry);
    }

    std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) {
        return a.last_write_time() < b.last_write_time();
        });

    while (files.size() > 5) 
    {
        std::filesystem::remove(files.front().path());
        files.erase(files.begin());
    }
}