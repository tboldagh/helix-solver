#include "Logger/LogMessage.h"

#include <chrono>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

class LogMessageSuite : public ::testing::Test
{
protected:
    const Logger::LogMessage::Severity severity_ = Logger::LogMessage::Severity::Debug;
    const std::string message_ = "Test message";
    const Logger::LogMessage::Timestamp timestamp_ = std::chrono::system_clock::now();
    const std::string file_ = "TestFile.cpp";
    const int line_ = 42;
    const std::string function_ = "testFunction";
};

TEST_F(LogMessageSuite, StringConstruct)
{
    const Logger::LogMessage logMessage(severity_, message_, timestamp_, file_, line_, function_);

    ASSERT_EQ(severity_, logMessage.getSeverity());
    ASSERT_EQ("DEBUG", logMessage.getSeverityString());
    ASSERT_EQ(message_, logMessage.getMessage());
    ASSERT_EQ(timestamp_, logMessage.getTimestamp());
    ASSERT_EQ(file_, logMessage.getFile());
    ASSERT_EQ(line_, logMessage.getLine());
    ASSERT_EQ(function_, logMessage.getFunction());
}

TEST_F(LogMessageSuite, StreamConstruct)
{
    std::stringstream messageStream;
    messageStream << message_;
    const Logger::LogMessage logMessage(severity_, messageStream, timestamp_, file_, line_, function_);

    ASSERT_EQ(severity_, logMessage.getSeverity());
    ASSERT_EQ("DEBUG", logMessage.getSeverityString());
    ASSERT_EQ(message_, logMessage.getMessage());
    ASSERT_EQ(timestamp_, logMessage.getTimestamp());
    ASSERT_EQ(file_, logMessage.getFile());
    ASSERT_EQ(line_, logMessage.getLine());
    ASSERT_EQ(function_, logMessage.getFunction());
}
