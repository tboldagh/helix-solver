#pragma once

#include <string>
#include <sstream>
#include <chrono>

namespace Logger
{
class LogMessage
{
public:
    enum class Severity : u_int8_t
    {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    using Timestamp = std::chrono::time_point<std::chrono::system_clock>;

    inline LogMessage(Severity severity, std::string message, Timestamp timestamp, std::string file, int line, std::string function);
    inline LogMessage(Severity severity, const std::stringstream& message, Timestamp timestamp, std::string file, int line, std::string function);

    inline Severity getSeverity() const;
    inline std::string getSeverityString() const;
    inline const std::string& getMessage() const;
    inline const Timestamp& getTimestamp() const;
    inline const std::string& getFile() const;
    inline int getLine() const;
    inline const std::string& getFunction() const;

private:
    const Severity severity_;
    const std::string message_;
    const Timestamp timestamp_;
    const std::string file_;
    const int line_;
    const std::string function_;
};

LogMessage::LogMessage(Severity severity, std::string message, Timestamp timestamp, std::string file, int line, std::string function)
    : severity_(severity)
    , message_(std::move(message))
    , timestamp_(std::move(timestamp))
    , file_(std::move(file))
    , line_(line)
    , function_(std::move(function)) {}

LogMessage::LogMessage(Severity severity, const std::stringstream& message, Timestamp timestamp, std::string file, int line, std::string function)
    : severity_(severity)
    , message_(message.str())
    , timestamp_(std::move(timestamp))
    , file_(std::move(file))
    , line_(line)
    , function_(std::move(function)) {}

LogMessage::Severity LogMessage::getSeverity() const
{
    return severity_;
}

std::string LogMessage::getSeverityString() const
{
    switch (severity_)
    {
        case Severity::Debug:
            return "DEBUG";
        case Severity::Info:
            return "INFO";
        case Severity::Warning:
            return "WARNING";
        case Severity::Error:
            return "ERROR";
        case Severity::Fatal:
            return "FATAL";
    }
}

const std::string& LogMessage::getMessage() const
{
    return message_;
}

const LogMessage::Timestamp& LogMessage::getTimestamp() const
{
    return timestamp_;
}

const std::string& LogMessage::getFile() const
{
    return file_;
}

int LogMessage::getLine() const
{
    return line_;
}

const std::string& LogMessage::getFunction() const
{
    return function_;
}
} // namespace Logger
