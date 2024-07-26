#include "Logger/LogMessage.h"

namespace Logger
{
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
} // namespace Logger