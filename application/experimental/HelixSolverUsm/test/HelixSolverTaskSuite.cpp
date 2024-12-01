#include "HelixSolverUsm/HelixSolverTask.h"
#include "SplitterUsm/SplitterSettings.h"
#include "SplitterUsm/Splitter.h"
#include "EventUsm/EventUsm.h"
#include "Logger/Logger.h"
#include "Logger/OstreamLogger.h"
#include "SplitterTest/SplitterTest.h"
#include "ITaskStateObserverMock/ITaskStateObserverMock.h"
#include "IQueueMock/IQueueMock.h"

#include <gtest/gtest.h>
#include <iostream>
#include <CL/sycl.hpp>


class HelixSolverTaskTest : public SplitterTest
{
protected:
    HelixSolverTaskTest()
    : logger_(std::cout)
    , event_(std::make_unique<EventUsm>(eventId_))
    , result_(std::make_unique<ResultUsm>(resultId_))
    , eventPtr_(event_.get())
    , resultPtr_(result_.get())
    {
        Logger::ILogger::setGlobalInstance(&logger_);

        task_.takeEventAndResult(std::move(event_), std::move(result_));

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.onAssignedToWorker(stateObserverMock_);

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        EXPECT_CALL(queueMock_, getQueue()).WillRepeatedly(testing::ReturnRef(syclQueue_));
        task_.assignQueue(queueMock_);

        prepareEventResources();
    }

    ~HelixSolverTaskTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);

        freeEventResources();
    }

    void prepareEventResources()
    {
        eventResources_ = EventUsm::allocateDeviceResources(syclQueue_);
        eventResources_->emplace(DeviceResourceType::Splitter, sycl::malloc_device<Splitter>(1, syclQueue_));
        syclQueue_.memcpy(eventResources_->at(DeviceResourceType::Splitter), &splitter_, sizeof(Splitter)).wait();
    }

    void prepareResultResources()
    {
        resultResources_ = ResultUsm::allocateDeviceResources(syclQueue_);
    }

    void freeEventResources()
    {
        EventUsm::deallocateDeviceResources(*eventResources_, syclQueue_);
        sycl::free(eventResources_->at(DeviceResourceType::Splitter), syclQueue_);
    }

    void freeResultResources()
    {
        ResultUsm::deallocateDeviceResources(*resultResources_, syclQueue_);
    }

    Logger::OstreamLogger logger_;

    static constexpr EventUsm::EventId eventId_ = 21;
    static constexpr ResultUsm::ResultId resultId_ = 37;
    static constexpr ITask::TaskId taskId_ = 42;
    static constexpr IQueue::DeviceResourceGroupId eventResourceGroupId_ = 43;
    static constexpr IQueue::DeviceResourceGroupId resultResourceGroupId_ = 44;

    HelixSolverTask task_{taskId_, splitter_};

    sycl::queue syclQueue_{sycl::gpu_selector_v};
    ITaskStateObserverMock stateObserverMock_;
    IQueueMock queueMock_;

    std::unique_ptr<EventUsm> event_;
    std::unique_ptr<ResultUsm> result_;
    EventUsm* eventPtr_;
    ResultUsm* resultPtr_;

    std::unique_ptr<DeviceResourceGroup> eventResources_;
    std::unique_ptr<DeviceResourceGroup> resultResources_;
};

TEST_F(HelixSolverTaskTest, TakeEventResources)
{
    task_.takeEventResources(std::make_pair(eventResourceGroupId_, *eventResources_));

    ASSERT_TRUE(task_.isEventResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);
    ASSERT_EQ(eventPtr_->deviceNumPoints_, eventResources_->at(DeviceResourceType::NumPoints));
    ASSERT_EQ(eventPtr_->deviceXs_, eventResources_->at(DeviceResourceType::Xs));
    ASSERT_EQ(eventPtr_->deviceYs_, eventResources_->at(DeviceResourceType::Ys));
    ASSERT_EQ(eventPtr_->deviceZs_, eventResources_->at(DeviceResourceType::Zs));
    ASSERT_EQ(eventPtr_->deviceLayers_, eventResources_->at(DeviceResourceType::Layers));
    ASSERT_EQ(task_.deviceSplitter_, eventResources_->at(DeviceResourceType::Splitter));
}
