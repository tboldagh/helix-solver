#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"
#include "IQueueMock/IQueueMock.h"
#include "EventUsm/QueueUsm.h"

#include <sycl/sycl.hpp>
#include <gtest/gtest.h>


class TestKernelMemory : public KernelMemory
{
public:
    TestKernelMemory(sycl::queue& syclQueue)
    : KernelMemory(syclQueue)
    {
        allocate();
    }

    ~TestKernelMemory()
    {
        deallocate();
    }

protected:
    void allocateInternal() override
    {
        testMemory_ = sycl::malloc_device<u_int32_t>(TestMemorySize, queue_);
    }

    void deallocateInternal() override
    {
        sycl::free(testMemory_, queue_);
    }

    static constexpr u_int32_t TestMemorySize{42};
    u_int32_t* testMemory_;
};

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
        // TODO check message properties

        // EXPECT_CALL(loggerMock_, log(testing::AllOf(
        //     testing::Property(&Logger::LogMessage::getSeverity, severity),
        //     testing::Property(&Logger::LogMessage::getMessage, testing::StrEq(message))
        // )));
    }

    Logger::ILoggerMock loggerMock_;
    sycl::queue syclQueue_ = sycl::queue(sycl::gpu_selector_v);
    static constexpr IQueue::Capacity ResourcesCapacity{3};
    static constexpr IQueue::Capacity WorkCapacity{4};
    QueueUsm queueUsm_{syclQueue_, ResourcesCapacity, WorkCapacity};

    IQueue::CreateResourceGroupFunction createResourceGroupFunction_ = [](sycl::queue& syclQueue) -> std::unique_ptr<DeviceResourceGroup> {
        TestKernelMemory* eventMemory = new TestKernelMemory(syclQueue);
        TestKernelMemory* resultMemory = new TestKernelMemory(syclQueue);
        return std::make_unique<DeviceResourceGroup>(DeviceResourceGroup{
            {DeviceResourceType::EventKernelMemory, eventMemory},
            {DeviceResourceType::ResultKernelMemory, resultMemory}
        });
    };
};

TEST_F(QueueUsmTest, CreateResources)
{
    ASSERT_TRUE(queueUsm_.createResources(createResourceGroupFunction_));
    ASSERT_EQ(queueUsm_.getResourcesCapacity(), ResourcesCapacity);
    for (IQueue::Capacity i = 0; i < ResourcesCapacity; ++i)
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        ASSERT_NE(resourceGroupId, QueueUsm::NullResourceGroupId);
        ASSERT_NE(&resourceGroup, &QueueUsm::NullResourceGroup);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::EventKernelMemory), nullptr);
        ASSERT_NE(resourceGroup.at(DeviceResourceType::ResultKernelMemory), nullptr);
    }
    auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
    ASSERT_EQ(resourceGroupId, QueueUsm::NullResourceGroupId);
    ASSERT_EQ(&resourceGroup, &QueueUsm::NullResourceGroup);
}


TEST_F(QueueUsmTest, ReturnResourceGroup)
{
    ASSERT_TRUE(queueUsm_.createResources(createResourceGroupFunction_));
    std::vector<IQueue::DeviceResourceGroupId> resourceGroupIds;
    std::vector<DeviceResourceGroup> resourceGroups;
    for (IQueue::Capacity i = 0; i < ResourcesCapacity; ++i)
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        resourceGroupIds.push_back(resourceGroupId);
        resourceGroups.push_back(resourceGroup);
    }
    
    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        ASSERT_EQ(resourceGroupId, QueueUsm::NullResourceGroupId);
        ASSERT_EQ(&resourceGroup, &QueueUsm::NullResourceGroup);
    }

    queueUsm_.returnResources(resourceGroupIds[1]);
    queueUsm_.returnResources(resourceGroupIds[2]);
    queueUsm_.returnResources(resourceGroupIds[0]);

    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        ASSERT_EQ(resourceGroupId, resourceGroupIds[1]);
    }

    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        ASSERT_EQ(resourceGroupId, resourceGroupIds[2]);
    }

    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        ASSERT_EQ(resourceGroupId, resourceGroupIds[0]);
    }

    {
        auto [resourceGroupId, resourceGroup] = queueUsm_.getResources();
        ASSERT_EQ(resourceGroupId, QueueUsm::NullResourceGroupId);
        ASSERT_EQ(&resourceGroup, &QueueUsm::NullResourceGroup);
    }
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
    ASSERT_EQ(0, queueUsm_.getWorkLoad());

    queueUsm_.incrementWorkLoad();
    ASSERT_EQ(1, queueUsm_.getWorkLoad());
}

