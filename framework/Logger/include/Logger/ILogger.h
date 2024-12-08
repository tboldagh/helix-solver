#pragma once

#include <Logger/LogMessage.h>

namespace Logger
{
class ILogger
{
public:
    ILogger() = default;
    ILogger(const ILogger&) = delete;
    virtual ~ILogger() = default;
    ILogger& operator=(const ILogger&) = delete;

    virtual void log(LogMessage&& message) = 0;
    virtual void setMinSeverity(LogMessage::Severity minSeverity) = 0;
    
    static ILogger* getGlobalInstance();
    static void setGlobalInstance(ILogger* instance);

private:
    static ILogger* globalInstance_;
};
} // namespace Logger