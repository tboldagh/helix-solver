#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"
#include "IQueueMock/IQueueMock.h"
#include "EventUsm/QueueUsm.h"

#include <CL/sycl.hpp>
#include <gtest/gtest.h>

class QueueUsmTest : public ::testing::Test
{
protected:
    QueueUsmTest()
    {
        Logger::ILogger::setGlobalInstance(&loggerMock_);
    }

    ~QueueUsmTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    void expectLog(const Logger::LogMessage::Severity severity, const std::string& message)
    {
        EXPECT_CALL(loggerMock_, log(testing::AllOf(
            testing::Property(&Logger::LogMessage::getSeverity, severity),
            testing::Property(&Logger::LogMessage::getMessage, testing::StrEq(message))
        )));
    }

    Logger::ILoggerMock loggerMock_;
    sycl::queue syclQueue_ = sycl::queue(sycl::gpu_selector_v);
    static constexpr IQueue::Capacity EventResourcesCapacity{3};
    static constexpr IQueue::Capacity ResultResourcesCapacity{4};
    static constexpr IQueue::Capacity WorkCapacity{2};
    QueueUsm queueUsm_{syclQueue_, EventResourcesCapacity, ResultResourcesCapacity, WorkCapacity};
};

TEST_F(QueueUsmTest, GetEventResourceGroup)
{
    for (IQueue::Capacity i = 0; i < EventResourcesCapacity; ++i)
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getEventResourceGroup();
        ASSERT_NE(resourceGroupId, QueueUsm::NullEventResourceGroupId);
        ASSERT_NE(&resourceGroup, &QueueUsm::NullResourceGroup);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::NumPoints), nullptr);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::Xs), nullptr);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::Ys), nullptr);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::Zs), nullptr);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::Layers), nullptr);
    }
    ASSERT_EQ(queueUsm_.getEventResourcesLoad(), queueUsm_.getEventResourcesCapacity());
}

TEST_F(QueueUsmTest, GetEventResourceGroupWhenNoResourcesAvailable)
{
    for (IQueue::Capacity i = 0; i < EventResourcesCapacity; ++i)
    {
        queueUsm_.getEventResourceGroup();
    }
    ASSERT_EQ(queueUsm_.getEventResourcesLoad(), queueUsm_.getEventResourcesCapacity());

    expectLog(Logger::LogMessage::Severity::Error, "No free event resources available in the queue.");

    auto [resourceGroupId, resourceGroup] = queueUsm_.getEventResourceGroup();
    ASSERT_EQ(resourceGroupId, QueueUsm::NullEventResourceGroupId);
    ASSERT_EQ(&resourceGroup, &QueueUsm::NullResourceGroup);
    ASSERT_EQ(queueUsm_.getEventResourcesLoad(), queueUsm_.getEventResourcesCapacity());
}

TEST_F(QueueUsmTest, ReturnEventResourceGroup)
{
    std::vector<IQueue::DeviceResourceGroupId> resourceGroupIds;
    for (IQueue::Capacity i = 0; i < EventResourcesCapacity; ++i)
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getEventResourceGroup();
        resourceGroupIds.push_back(resourceGroupId);
    }
    ASSERT_EQ(queueUsm_.getEventResourcesLoad(), queueUsm_.getEventResourcesCapacity());

    for (const auto& resourceGroupId : resourceGroupIds)
    {
        queueUsm_.returnEventResourceGroup(resourceGroupId);
    }
    ASSERT_EQ(queueUsm_.getEventResourcesLoad(), 0);
}

TEST_F(QueueUsmTest, GetResultResourceGroup)
{
    for (IQueue::Capacity i = 0; i < ResultResourcesCapacity; ++i)
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResultResourceGroup();
        ASSERT_NE(resourceGroupId, QueueUsm::NullResultResourceGroupId);
        ASSERT_NE(&resourceGroup, &QueueUsm::NullResultResourceGroup);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::SomeSolutionParameters), nullptr);
    }
    ASSERT_EQ(queueUsm_.getResultResourcesLoad(), queueUsm_.getResultResourcesCapacity());
}

TEST_F(QueueUsmTest, GetResultResourceGroupWhenNoResourcesAvailable)
{
    for (IQueue::Capacity i = 0; i < ResultResourcesCapacity; ++i)
    {
        queueUsm_.getResultResourceGroup();
    }
    ASSERT_EQ(queueUsm_.getResultResourcesLoad(), queueUsm_.getResultResourcesCapacity());

    expectLog(Logger::LogMessage::Severity::Error, "No free result resources available in the queue.");

    auto [resourceGroupId, resourceGroup] = queueUsm_.getResultResourceGroup();
    ASSERT_EQ(resourceGroupId, QueueUsm::NullResultResourceGroupId);
    ASSERT_EQ(&resourceGroup, &QueueUsm::NullResultResourceGroup);
    ASSERT_EQ(queueUsm_.getResultResourcesLoad(), queueUsm_.getResultResourcesCapacity());
}

TEST_F(QueueUsmTest, ReturnResultResourceGroup)
{
    std::vector<IQueue::DeviceResourceGroupId> resourceGroupIds;
    for (IQueue::Capacity i = 0; i < ResultResourcesCapacity; ++i)
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResultResourceGroup();
        resourceGroupIds.push_back(resourceGroupId);
    }
    ASSERT_EQ(queueUsm_.getResultResourcesLoad(), queueUsm_.getResultResourcesCapacity());

    for (const auto& resourceGroupId : resourceGroupIds)
    {
        queueUsm_.returnResultResourceGroup(resourceGroupId);
    }
    ASSERT_EQ(queueUsm_.getResultResourcesLoad(), 0);
}

TEST_F(QueueUsmTest, IncrementAndDecrementWorkLoad)
{
    ASSERT_EQ(queueUsm_.getWorkLoad(), 0);

    while (queueUsm_.getWorkLoad() < queueUsm_.getWorkCapacity())
    {
        queueUsm_.incrementWorkLoad();
    }
    ASSERT_EQ(WorkCapacity, queueUsm_.getWorkLoad());

    while (queueUsm_.getWorkLoad() > 0)
    {
        queueUsm_.decrementWorkLoad();
    }
}

