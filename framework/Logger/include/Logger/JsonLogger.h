#pragma once

#include "Logger/ILogger.h"
#include "Logger/LogMessage.h"

#include <fstream>
#include <string>
#include <filesystem>

namespace Logger
{
class JsonLogger : public ILogger
{
public:
    JsonLogger(std::string filePath);
    ~JsonLogger() override;

    void log(LogMessage&& message) override;
private:
    void createDirectoryIfNotExists();

    const std::filesystem::path filePath_;
    std::ofstream file_;
};
} // namespace Logger