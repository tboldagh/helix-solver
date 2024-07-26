#include "Logger/LogMessage.h"
#include "Logger/JsonLogger.h"

#include <chrono>
#include <gtest/gtest.h>
#include <ostream>
#include <sstream>
#include <string>
#include <fstream>

class JsonLoggerSuite : public ::testing::Test
{
protected:
    JsonLoggerSuite()
    {
        const char* sandboxDir = std::getenv("TEST_SANDBOX_DIR");
        if (sandboxDir == nullptr)
        {
            throw std::runtime_error("Environment variable $TEST_SANDBOX_DIR not set");
        }

        filePath_ = std::string(sandboxDir) + "/JsonLoggerSuite/log.json";

        std::filesystem::path directoryPath{filePath_};
        directoryPath.remove_filename();
        if (!std::filesystem::exists(directoryPath))
        {
            std::filesystem::create_directories(directoryPath);
        }
    }

    ~JsonLoggerSuite()
    {
        std::filesystem::path directoryPath{filePath_};
        directoryPath.remove_filename();
        std::filesystem::remove_all(directoryPath);
    }

    std::string readLogFile()
    {
        std::ifstream logFile(filePath_);

        std::string log;
        logFile.seekg(0, std::ios::end);
        log.reserve(logFile.tellg());
        logFile.seekg(0, std::ios::beg);
        log.assign((std::istreambuf_iterator<char>(logFile)), std::istreambuf_iterator<char>());

        logFile.close();

        return log;
    }

    std::string filePath_;

    static const Logger::LogMessage::Severity severity;
    static const std::string message;
    static const Logger::LogMessage::Timestamp timestamp;
    static const std::string file;
    static const int line;
    static const std::string function;
    static const Logger::LogMessage logMessage;
    static const std::string expectedLog;
};

const Logger::LogMessage::Severity JsonLoggerSuite::severity = Logger::LogMessage::Severity::Debug;
const std::string JsonLoggerSuite::message = "Test message";
const Logger::LogMessage::Timestamp JsonLoggerSuite::timestamp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::microseconds(1112477862123456));
const std::string JsonLoggerSuite::file = "TestFile.cpp";
const int JsonLoggerSuite::line = 42;
const std::string JsonLoggerSuite::function = "testFunction";
const Logger::LogMessage JsonLoggerSuite::logMessage{severity, message, timestamp, file, line, function};
const std::string JsonLoggerSuite::expectedLog = 
    "{\n"
    "\t\"timestamp\": \"2005-04-02 21:37:42.123456\",\n"
    "\t\"file\": \"TestFile.cpp\",\n"
    "\t\"line\": 42,\n"
    "\t\"function\": \"testFunction\",\n"
    "\t\"severity\": \"DEBUG\",\n"
    "\t\"message\": \"Test message\"\n"
    "},";

TEST_F(JsonLoggerSuite, LogSingleMessage)
{
    Logger::JsonLogger* logger = new Logger::JsonLogger(filePath_);
    Logger::ILogger::setGlobalInstance(logger);

    Logger::LogMessage messageCopy = logMessage;
    logger->log(std::move(messageCopy));
    
    Logger::ILogger::setGlobalInstance(nullptr);
    delete logger;

    std::string log = readLogFile();

    // File starts with [
    ASSERT_EQ('[', log[0]);
    
    const auto logStart = log.find(expectedLog);
    ASSERT_NE(std::string::npos, logStart);
    // No duplicates
    const auto logEnd = logStart + expectedLog.size();
    ASSERT_EQ(std::string::npos, log.find(expectedLog, logEnd));

    // File ends with ]
    ASSERT_EQ(']', log[log.size() - 2]);
}

TEST_F(JsonLoggerSuite, LogSingleMessageDirectoryNotExists)
{
    std::filesystem::path directoryPath{filePath_};
    directoryPath.remove_filename();
    std::filesystem::remove_all(directoryPath);

    Logger::JsonLogger* logger = new Logger::JsonLogger(filePath_);
    Logger::ILogger::setGlobalInstance(logger);

    Logger::LogMessage messageCopy = logMessage;
    logger->log(std::move(messageCopy));
    
    Logger::ILogger::setGlobalInstance(nullptr);
    delete logger;

    std::string log = readLogFile();

    // File starts with [
    ASSERT_EQ('[', log[0]);
    
    const auto logStart = log.find(expectedLog);
    ASSERT_NE(std::string::npos, logStart);

    // No duplicates
    const auto logEnd = logStart + expectedLog.size();
    ASSERT_EQ(std::string::npos, log.find(expectedLog, logEnd));

    // File ends with ]
    ASSERT_EQ(']', log[log.size() - 2]);
}

TEST_F(JsonLoggerSuite, LogMultipleMessages)
{
    Logger::JsonLogger* logger = new Logger::JsonLogger(filePath_);
    Logger::ILogger::setGlobalInstance(logger);

    Logger::LogMessage messageCopy = logMessage;
    logger->log(std::move(messageCopy));
    logger->log(std::move(messageCopy));
    
    Logger::ILogger::setGlobalInstance(nullptr);
    delete logger;

    std::string log = readLogFile();

    // File starts with [
    ASSERT_EQ('[', log[0]);
    
    // Two messages
    const auto logStart = log.find(expectedLog);
    ASSERT_NE(std::string::npos, logStart);
    const auto logEnd = logStart + expectedLog.size();
    const auto secondLogStart = log.find(expectedLog, logEnd);
    ASSERT_NE(std::string::npos, secondLogStart);

    // No more duplicates
    const auto secondLogEnd = secondLogStart + expectedLog.size();
    ASSERT_EQ(std::string::npos, log.find(expectedLog, secondLogEnd));

    // File ends with ]
    ASSERT_EQ(']', log[log.size() - 2]);
}