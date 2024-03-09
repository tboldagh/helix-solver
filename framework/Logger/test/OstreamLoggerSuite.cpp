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

    const Logger::LogMessage::Severity severity_ = Logger::LogMessage::Severity::Debug;
    const std::string message_ = "Test message";
    const Logger::LogMessage::Timestamp timestamp_ = std::chrono::time_point<std::chrono::system_clock>(std::chrono::microseconds(1112477862123456));
    const std::string file_ = "TestFile.cpp";
    const int line_ = 42;
    const std::string function_ = "testFunction";
    const Logger::LogMessage logMessage_{severity_, message_, timestamp_, file_, line_, function_};
    const std::string expectedLog_ = "2005-04-02 21:37:42.123456\t" + file_ + ":42\t" + function_ + "\tDEBUG\t" + message_ + "\n";
};

TEST_F(OstreamLoggerSuite, LogMove)
{
    Logger::LogMessage messageCopy = logMessage_;
    logger_.log(std::move(messageCopy));

    ASSERT_EQ(expectedLog_, ostream_.str());
}