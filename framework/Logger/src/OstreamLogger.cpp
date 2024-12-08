#include "Logger/OstreamLogger.h"

#include <ctime>
#include <iomanip>

namespace Logger
{
OstreamLogger::OstreamLogger(std::ostream& stream)
    : ostream_(stream) {}

void OstreamLogger::log(LogMessage&& message)
{
    if (message.getSeverity() < minSeverity_)
    {
        return;
    }

    std::chrono::microseconds microsecondsSinceEpoch{std::chrono::time_point_cast<std::chrono::microseconds>(message.getTimestamp()).time_since_epoch()};
    std::time_t seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::microseconds(microsecondsSinceEpoch)).count();
    std::time_t microseconds = std::chrono::microseconds(microsecondsSinceEpoch % 1000000).count();

    ostream_ << std::put_time(std::gmtime(&seconds), "%Y-%m-%d %H:%M:%S")
                << "." << std::setfill('0') << std::setw(6) << microseconds;

    ostream_ << "\t" << message.getFile();
    ostream_ << ":" << message.getLine();
    ostream_ << "\t" << message.getFunction();
    ostream_ << "\t" << message.getSeverityString();
    ostream_ << "\t" << message.getMessage();
    ostream_ << "\n";
}

void OstreamLogger::setMinSeverity(LogMessage::Severity minSeverity)
{
    minSeverity_ = minSeverity;
}
} // namespace Logger