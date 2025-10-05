#include "RootEventLoader/RootEventLoader.h"
#include "Logger/ILogger.h"
#include "Logger/OstreamLogger.h"
#include "Logger/Logger.h"
#include "EventUsm/EventUsm.h"

#include <gtest/gtest.h>
#include <iostream>

class RootEventLoaderTest : public ::testing::Test
{
protected:
    RootEventLoaderTest()
    {
        Logger::ILogger::setGlobalInstance(&logger_);

        EXPECT_TRUE(loader_.setInputFile(InputFilePath));
    }

    ~RootEventLoaderTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    Logger::OstreamLogger logger_{std::cout};

    static const std::string InputFilePath;
    RootEventLoader loader_;
};

const std::string RootEventLoaderTest::InputFilePath = "/helix/repo/data/odd_output_singleMu_1000/spacepoints.root";

TEST_F(RootEventLoaderTest, GetEventIds)
{
    const std::vector<u_int32_t>& eventIds = loader_.getEventIds();
    EXPECT_GT(eventIds.size(), 0);
}

TEST_F(RootEventLoaderTest, LoadEvent)
{
    auto [success, event] = loader_.loadEvent(0);
    EXPECT_TRUE(success);
    EXPECT_EQ(event->eventId_, 0);

    for (u_int32_t i = 0; i < event->hostNumPoints_; i++)
    {
        EXPECT_NE(event->hostXs_[i], 0);
        EXPECT_NE(event->hostYs_[i], 0);
        EXPECT_NE(event->hostZs_[i], 0);
    }
}

TEST_F(RootEventLoaderTest, LoadAllEvents)
{
    const auto events = loader_.loadAllEvents();
    EXPECT_NE(events, nullptr);
    EXPECT_GT(events->size(), 0);

    for (const auto& [eventId, event] : *events)
    {
        EXPECT_NE(event->hostXs_, nullptr);
        EXPECT_NE(event->hostYs_, nullptr);
        EXPECT_NE(event->hostZs_, nullptr);
    }

    const auto& event = events->at(42);
    for (u_int32_t i = 0; i < event->hostNumPoints_; i++)
    {
        EXPECT_NE(event->hostXs_[i], 0);
        EXPECT_NE(event->hostYs_[i], 0);
        EXPECT_NE(event->hostZs_[i], 0);
    }
}