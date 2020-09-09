#pragma once

#include <iostream>
#include <mutex>

class Logger
{
private:
    std::unique_ptr<std::ostream> m_combinedStream;
    std::unique_ptr<std::ostream> m_errorStream;
    std::mutex m_logLock;

public:
    Logger() = default;

    void debug(std::string msg);
    void error(std::string msg);
    void info(std::string msg);
    void log(std::string level, std::string msg);
    void warn(std::string msg);
};

