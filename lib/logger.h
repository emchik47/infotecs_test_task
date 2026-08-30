#pragma once

#include <ctime>
#include <fstream>
#include <iomanip>
#include <string>

enum class LogLevel{
    INFO = 0,
    WARNING = 1,
    ERROR = 2
};

class ILogger{
public:
    virtual ~ILogger() = default;

    virtual void SetLevel(LogLevel new_level) = 0;
    virtual LogLevel GetLevel() const = 0;
    virtual void Log(const std::string& text, LogLevel msg_level) = 0;

protected:
    std::string FormatMessage(const std::string& text, LogLevel msg_level) const;

};

class FileLogger : public ILogger{
private:
    LogLevel level;
    std::string filename;
    std::ofstream file_stream;

public:
    FileLogger(const std::string& filename_input, LogLevel default_level);
    
    ~FileLogger() override;

    void SetLevel(LogLevel new_level) override;
    LogLevel GetLevel() const override;
    void Log(const std::string& text, LogLevel msg_level) override;

};

class SocketLogger : public ILogger{
private:
    LogLevel level;
    std::string ip_address;
    int descriptor;

public:
    SocketLogger(const std::string& ip, int port, LogLevel default_level);

    ~SocketLogger() override;

    void SetLevel(LogLevel new_level) override;
    LogLevel GetLevel() const override;
    void Log(const std::string& text, LogLevel msg_level) override;
};