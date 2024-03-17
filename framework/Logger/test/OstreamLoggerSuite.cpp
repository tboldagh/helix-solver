#include "Logger/LogMessage.h"
#include "Logger/OstreamLogger.h"

#include <chrono>
#include <gtest/gtest.h>
#include <ostream>
#include <sstream>
#include <string>

class OstreamLoggerSuite : public ::testing::Test
{
protected:
    OstreamLoggerSuite()
        : ostream_()
        , logger_(ostream_) {}

    std::ostringstream ostream_;
    Logger::OstreamLogger logger_;

    static const Logger::LogMessage::Severity severity;
    static const std::string message;
    static const Logger::LogMessage::Timestamp timestamp;
    static const std::string file;
    static const int line;
    static const std::string function;
    static const Logger::LogMessage logMessage;
    static const std::string expectedLog;
};

const Logger::LogMessage::Severity OstreamLoggerSuite::severity = Logger::LogMessage::Severity::Debug;
const std::string OstreamLoggerSuite::message = "Test message";
const Logger::LogMessage::Timestamp OstreamLoggerSuite::timestamp = std::chrono::time_point<std::chrono::system_clock>(std::chrono::microseconds(1112477862123456));
const std::string OstreamLoggerSuite::file = "TestFile.cpp";
const int OstreamLoggerSuite::line = 42;
const std::string OstreamLoggerSuite::function = "testFunction";
const Logger::LogMessage OstreamLoggerSuite::logMessage{severity, message, timestamp, file, line, function};
const std::string OstreamLoggerSuite::expectedLog = "2005-04-02 21:37:42.123456\t" + file + ":42\t" + function + "\tDEBUG\t" + message + "\n";

TEST_F(OstreamLoggerSuite, LogMove)
{
    Logger::LogMessage messageCopy = logMessage;
    logger_.log(std::move(messageCopy));

    ASSERT_EQ(expectedLog, ostream_.str());
}