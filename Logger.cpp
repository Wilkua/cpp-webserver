#include <iostream>
#include <mutex>

#include "Logger.h"

void Logger::debug(std::string msg)
{
    log("DEBUG", msg);
}

void Logger::error(std::string msg)
{
    log("ERROR", msg);
}

void Logger::info(std::string msg)
{
    log("INFO", msg);
}

void Logger::log(std::string level, std::string msg)
{
    std::lock_guard<std::mutex> lg(m_logLock);

    if (m_combinedStream == nullptr)
    {
        std::cout
            << "(" << level << ") "
            << msg << std::endl;
    }
}

void Logger::warn(std::string msg)
{
    log("WARN", msg);
}

