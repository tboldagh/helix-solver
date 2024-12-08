#include "Logger/JsonLogger.h"
#include "Logger/Logger.h"

#include <ctime>
#include <iomanip>
#include <string>

namespace Logger
{
JsonLogger::JsonLogger(std::string filePath)
: filePath_{std::move(filePath)}
{
    createDirectoryIfNotExists();
    file_.open(filePath_, std::ios::out | std::ios::trunc);
    if (!file_.is_open())
    {
        return;
    }

    file_ << "[" << std::endl;
    log(LogMessage{LogMessage::Severity::Info, "JsonLogger started logging to file: " + std::string(filePath_), std::chrono::system_clock::now(), __FILE__, __LINE__, __func__});
}

JsonLogger::~JsonLogger()
{
    if (!file_.is_open())
    {
        return;
    }

    log(LogMessage{LogMessage::Severity::Info, "JsonLogger stopped logging to file: " + std::string(filePath_), std::chrono::system_clock::now(), __FILE__, __LINE__, __func__});
    file_ << "]" << std::endl;
    file_.close();
}

void JsonLogger::log(LogMessage&& message)
{
    if (message.getSeverity() < minSeverity_)
    {
        return;
    }

    std::chrono::microseconds microsecondsSinceEpoch{std::chrono::time_point_cast<std::chrono::microseconds>(message.getTimestamp()).time_since_epoch()};
    std::time_t seconds = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::microseconds(microsecondsSinceEpoch)).count();
    std::time_t microseconds = std::chrono::microseconds(microsecondsSinceEpoch % 1000000).count();

    file_ << "{" << std::endl;
    file_ << "\t\"timestamp\": \"" << std::put_time(std::gmtime(&seconds), "%Y-%m-%d %H:%M:%S")
                << "." << std::setfill('0') << std::setw(6) << microseconds << "\"," << std::endl;
    file_ << "\t\"file\": \"" << message.getFile() << "\"," << std::endl;
    file_ << "\t\"line\": " << message.getLine() << "," << std::endl;
    file_ << "\t\"function\": \"" << message.getFunction() << "\"," << std::endl;
    file_ << "\t\"severity\": \"" << message.getSeverityString() << "\"," << std::endl;
    file_ << "\t\"message\": \"" << message.getMessage() << "\"" << std::endl;
    file_ << "}," << std::endl;
}

void JsonLogger::setMinSeverity(LogMessage::Severity minSeverity)
{
    minSeverity_ = minSeverity;
}

void JsonLogger::createDirectoryIfNotExists()
{
    std::filesystem::path directoryPath{filePath_};
    directoryPath.remove_filename();

    if (!std::filesystem::exists(directoryPath))
    {
        std::filesystem::create_directories(directoryPath);
    }
}
} // namespace Logger