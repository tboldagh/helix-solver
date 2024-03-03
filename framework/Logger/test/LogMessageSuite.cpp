#include "Logger/LogMessage.h"

#include <gtest/gtest.h>
#include <sstream>
#include <string>

class LogMessageSuite : public ::testing::Test
{
protected:
    const Logger::LogMessage::Severity severity = Logger::LogMessage::Severity::Debug;
    const std::string message = "Test message";
    const Logger::LogMessage::Timestamp timestamp = std::chrono::system_clock::now();
    const std::string file = "TestFile.cpp";
    const int line = 42;
    const std::string function = "testFunction";
};

TEST_F(LogMessageSuite, StringConstruct)
{
    const Logger::LogMessage logMessage(severity, message, timestamp, file, line, function);

    ASSERT_EQ(severity, logMessage.getSeverity());
    ASSERT_EQ("DEBUG", logMessage.getSeverityString());
    ASSERT_EQ(message, logMessage.getMessage());
    ASSERT_EQ(timestamp, logMessage.getTimestamp());
    ASSERT_EQ(file, logMessage.getFile());
    ASSERT_EQ(line, logMessage.getLine());
    ASSERT_EQ(function, logMessage.getFunction());
}

TEST_F(LogMessageSuite, StreamConstruct)
{
    std::stringstream messageStream;
    messageStream << message;
    const Logger::LogMessage logMessage(severity, messageStream, timestamp, file, line, function);

    ASSERT_EQ(severity, logMessage.getSeverity());
    ASSERT_EQ("DEBUG", logMessage.getSeverityString());
    ASSERT_EQ(message, logMessage.getMessage());
    ASSERT_EQ(timestamp, logMessage.getTimestamp());
    ASSERT_EQ(file, logMessage.getFile());
    ASSERT_EQ(line, logMessage.getLine());
    ASSERT_EQ(function, logMessage.getFunction());
}
