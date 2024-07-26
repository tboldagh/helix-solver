#include "ILoggerMock/ILoggerMock.h"
#include "Logger/Logger.h"
#include "Logger/LogMessage.h"

#include <gtest/gtest.h>

class LoggerTest : public ::testing::Test
{
protected:
    LoggerTest()
    {
        Logger::ILogger::setGlobalInstance(&loggerMock_);
    }

    ~LoggerTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    Logger::ILoggerMock loggerMock_;

    const Logger::LogMessage::Severity severity_ = Logger::LogMessage::Severity::Debug;
    const std::string message_ = "Test message";
    const Logger::LogMessage::Timestamp timestamp_ = std::chrono::time_point<std::chrono::system_clock>(std::chrono::microseconds(1112477862123456));
    const std::string file_ = "TestFile.cpp";
    const int line_ = 42;
    const std::string function_ = "testFunction";
    const Logger::LogMessage logMessage_{severity_, message_, timestamp_, file_, line_, function_};
};

TEST_F(LoggerTest, LogManualSeverity)
{
    std::unique_ptr<Logger::LogMessage> loggedMeessage;
    EXPECT_CALL(loggerMock_, log(testing::_)).WillOnce(testing::Invoke([&loggedMeessage](const Logger::LogMessage& message) {
        loggedMeessage = std::make_unique<Logger::LogMessage>(message);
    }));
    std::chrono::time_point<std::chrono::system_clock> timestampBefore = std::chrono::system_clock::now();
    LOG(severity_, message_);
    EXPECT_EQ(loggedMeessage->getLine(), __LINE__ - 1);
    EXPECT_EQ(loggedMeessage->getFile(), __FILE__);
    EXPECT_EQ(loggedMeessage->getFunction(), __func__);
    EXPECT_EQ(loggedMeessage->getMessage(), message_);
    EXPECT_EQ(loggedMeessage->getSeverity(), severity_);
    EXPECT_GE(loggedMeessage->getTimestamp(), timestampBefore);
    EXPECT_LE(loggedMeessage->getTimestamp(), std::chrono::system_clock::now());
}

TEST_F(LoggerTest, LogDebug)
{
    std::unique_ptr<Logger::LogMessage> loggedMeessage;
    EXPECT_CALL(loggerMock_, log(testing::_)).WillOnce(testing::Invoke([&loggedMeessage](const Logger::LogMessage& message) {
        loggedMeessage = std::make_unique<Logger::LogMessage>(message);
    }));
    std::chrono::time_point<std::chrono::system_clock> timestampBefore = std::chrono::system_clock::now();
    LOG_DEBUG(message_);
    EXPECT_EQ(loggedMeessage->getLine(), __LINE__ - 1);
    EXPECT_EQ(loggedMeessage->getFile(), __FILE__);
    EXPECT_EQ(loggedMeessage->getFunction(), __func__);
    EXPECT_EQ(loggedMeessage->getMessage(), message_);
    EXPECT_EQ(loggedMeessage->getSeverity(), Logger::LogMessage::Severity::Debug);
    EXPECT_GE(loggedMeessage->getTimestamp(), timestampBefore);
    EXPECT_LE(loggedMeessage->getTimestamp(), std::chrono::system_clock::now());
}

TEST_F(LoggerTest, LogInfo)
{
    std::unique_ptr<Logger::LogMessage> loggedMeessage;
    EXPECT_CALL(loggerMock_, log(testing::_)).WillOnce(testing::Invoke([&loggedMeessage](const Logger::LogMessage& message) {
        loggedMeessage = std::make_unique<Logger::LogMessage>(message);
    }));
    std::chrono::time_point<std::chrono::system_clock> timestampBefore = std::chrono::system_clock::now();
    LOG_INFO(message_);
    EXPECT_EQ(loggedMeessage->getLine(), __LINE__ - 1);
    EXPECT_EQ(loggedMeessage->getFile(), __FILE__);
    EXPECT_EQ(loggedMeessage->getFunction(), __func__);
    EXPECT_EQ(loggedMeessage->getMessage(), message_);
    EXPECT_EQ(loggedMeessage->getSeverity(), Logger::LogMessage::Severity::Info);
    EXPECT_GE(loggedMeessage->getTimestamp(), timestampBefore);
    EXPECT_LE(loggedMeessage->getTimestamp(), std::chrono::system_clock::now());
}

TEST_F(LoggerTest, LogWarning)
{
    std::unique_ptr<Logger::LogMessage> loggedMeessage;
    EXPECT_CALL(loggerMock_, log(testing::_)).WillOnce(testing::Invoke([&loggedMeessage](const Logger::LogMessage& message) {
        loggedMeessage = std::make_unique<Logger::LogMessage>(message);
    }));
    std::chrono::time_point<std::chrono::system_clock> timestampBefore = std::chrono::system_clock::now();
    LOG_WARNING(message_);
    EXPECT_EQ(loggedMeessage->getLine(), __LINE__ - 1);
    EXPECT_EQ(loggedMeessage->getFile(), __FILE__);
    EXPECT_EQ(loggedMeessage->getFunction(), __func__);
    EXPECT_EQ(loggedMeessage->getMessage(), message_);
    EXPECT_EQ(loggedMeessage->getSeverity(), Logger::LogMessage::Severity::Warning);
    EXPECT_GE(loggedMeessage->getTimestamp(), timestampBefore);
    EXPECT_LE(loggedMeessage->getTimestamp(), std::chrono::system_clock::now());
}

TEST_F(LoggerTest, LogError)
{
    std::unique_ptr<Logger::LogMessage> loggedMeessage;
    EXPECT_CALL(loggerMock_, log(testing::_)).WillOnce(testing::Invoke([&loggedMeessage](const Logger::LogMessage& message) {
        loggedMeessage = std::make_unique<Logger::LogMessage>(message);
    }));
    std::chrono::time_point<std::chrono::system_clock> timestampBefore = std::chrono::system_clock::now();
    LOG_ERROR(message_);
    EXPECT_EQ(loggedMeessage->getLine(), __LINE__ - 1);
    EXPECT_EQ(loggedMeessage->getFile(), __FILE__);
    EXPECT_EQ(loggedMeessage->getFunction(), __func__);
    EXPECT_EQ(loggedMeessage->getMessage(), message_);
    EXPECT_EQ(loggedMeessage->getSeverity(), Logger::LogMessage::Severity::Error);
    EXPECT_GE(loggedMeessage->getTimestamp(), timestampBefore);
    EXPECT_LE(loggedMeessage->getTimestamp(), std::chrono::system_clock::now());
}

TEST_F(LoggerTest, LogFatal)
{
    std::unique_ptr<Logger::LogMessage> loggedMeessage;
    EXPECT_CALL(loggerMock_, log(testing::_)).WillOnce(testing::Invoke([&loggedMeessage](const Logger::LogMessage& message) {
        loggedMeessage = std::make_unique<Logger::LogMessage>(message);
    }));
    std::chrono::time_point<std::chrono::system_clock> timestampBefore = std::chrono::system_clock::now();
    LOG_FATAL(message_);
    EXPECT_EQ(loggedMeessage->getLine(), __LINE__ - 1);
    EXPECT_EQ(loggedMeessage->getFile(), __FILE__);
    EXPECT_EQ(loggedMeessage->getFunction(), __func__);
    EXPECT_EQ(loggedMeessage->getMessage(), message_);
    EXPECT_EQ(loggedMeessage->getSeverity(), Logger::LogMessage::Severity::Fatal);
    EXPECT_GE(loggedMeessage->getTimestamp(), timestampBefore);
    EXPECT_LE(loggedMeessage->getTimestamp(), std::chrono::system_clock::now());
}