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
#include <sycl/sycl.hpp>


class HelixSolverTaskInitTest : public SplitterTest
{
protected:
    HelixSolverTaskInitTest()
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
        // State: ReadyToQueue

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        EXPECT_CALL(queueMock_, getQueue()).WillRepeatedly(testing::ReturnRef(syclQueue_));
        task_.assignQueue(queueMock_);
        // State: WaitingForResources

        prepareEventResources();
        prepareResultResources();
    }

    ~HelixSolverTaskInitTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);

        freeEventResources();
        freeResultResources();
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

TEST_F(HelixSolverTaskInitTest, TakeEventResources)
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

class HelixSolverTaskExecutionTest : public HelixSolverTaskInitTest
{
protected:
    HelixSolverTaskExecutionTest()
    : HelixSolverTaskInitTest()
    {
        task_.takeEventResources(std::make_pair(eventResourceGroupId_, *eventResources_));
        // State: WaitingForResources

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.takeResultResources(std::make_pair(resultResourceGroupId_, *resultResources_));
        // State: WaitingForEventTransfer

        task_.isStateChanging_ = true;
        EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
        EXPECT_CALL(queueMock_, checkinQueue());
        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.transferEventToDeviceThread();
        // State: WaitingForExecution
    }
};

TEST_F(HelixSolverTaskExecutionTest, ExecuteOnDevice)
{
    task_.isStateChanging_ = true;
    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.executeThread();
    EXPECT_EQ(task_.getState(), ITask::State::Executed);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.releaseEventResourceGroup();
    EXPECT_EQ(task_.getState(), ITask::State::WaitingForResultTransfer);

    task_.isStateChanging_ = true;
    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.transferResultFromDeviceThread();
    EXPECT_EQ(task_.getState(), ITask::State::ResultTransferred);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.releaseResultResourceGroup();
    EXPECT_EQ(task_.getState(), ITask::State::Completed);
}

class HelixSolverTaskFullEvent : public SplitterTest
{
protected:
    HelixSolverTaskFullEvent()
    : logger_(std::cout)
    {
        Logger::ILogger::setGlobalInstance(&logger_);
    }

    ~HelixSolverTaskFullEvent() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    Logger::OstreamLogger logger_;

    static const std::string TestDataDir;
};
const std::string HelixSolverTaskFullEvent::TestDataDir = "/helix/repo/application/experimental/SplitterUsm/test-data";

TEST_F(HelixSolverTaskFullEvent, Basic)
{
    logger_.setMinSeverity(Logger::LogMessage::Severity::Debug);

    const std::string eventPath = TestDataDir + "/event_0.csv";
    constexpr EventUsm::EventId eventId = 42;
    std::optional<std::unique_ptr<EventUsm>> eventOptional = TestDataLoader::readEvent(eventPath, eventId);
    ASSERT_TRUE(eventOptional.has_value());
    std::unique_ptr<EventUsm> event = std::move(eventOptional.value());

    std::unique_ptr<ResultUsm> result = std::make_unique<ResultUsm>(eventId);

    static constexpr ITask::TaskId taskId = 21;
    HelixSolverTask task{taskId, splitter_};
    
    ITaskStateObserverMock stateObserverMock;
    IQueueMock queueMock;
    sycl::queue syclQueue{sycl::gpu_selector_v};

    task.takeEventAndResult(std::move(event), std::move(result));

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.onAssignedToWorker(stateObserverMock);
    EXPECT_EQ(task.getState(), ITask::State::ReadyToQueue);

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    EXPECT_CALL(queueMock, getQueue()).WillRepeatedly(testing::ReturnRef(syclQueue));
    task.assignQueue(queueMock);
    EXPECT_EQ(task.getState(), ITask::State::WaitingForResources);

    std::unique_ptr<DeviceResourceGroup> eventResources = EventUsm::allocateDeviceResources(syclQueue);
    eventResources->emplace(DeviceResourceType::Splitter, sycl::malloc_device<Splitter>(1, syclQueue));
    syclQueue.memcpy(eventResources->at(DeviceResourceType::Splitter), &splitter_, sizeof(Splitter)).wait();

    std::unique_ptr<DeviceResourceGroup> resultResources = ResultUsm::allocateDeviceResources(syclQueue);

    constexpr IQueue::DeviceResourceGroupId eventResourceGroupId = 43;
    task.takeEventResources(std::make_pair(eventResourceGroupId, *eventResources));
    EXPECT_EQ(task.getState(), ITask::State::WaitingForResources);

    constexpr IQueue::DeviceResourceGroupId resultResourceGroupId = 44;
    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.takeResultResources(std::make_pair(resultResourceGroupId, *resultResources));
    EXPECT_EQ(task.getState(), ITask::State::WaitingForEventTransfer);

    task.isStateChanging_ = true;
    EXPECT_CALL(queueMock, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue));
    EXPECT_CALL(queueMock, checkinQueue());
    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.transferEventToDeviceThread();
    EXPECT_EQ(task.getState(), ITask::State::WaitingForExecution);

    task.isStateChanging_ = true;
    EXPECT_CALL(queueMock, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue));
    EXPECT_CALL(queueMock, checkinQueue());
    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.executeThread();
    EXPECT_EQ(task.getState(), ITask::State::Executed);

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.releaseEventResourceGroup();
    EXPECT_EQ(task.getState(), ITask::State::WaitingForResultTransfer);

    task.isStateChanging_ = true;
    EXPECT_CALL(queueMock, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue));
    EXPECT_CALL(queueMock, checkinQueue());
    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.transferResultFromDeviceThread();
    EXPECT_EQ(task.getState(), ITask::State::ResultTransferred);

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.releaseResultResourceGroup();
    EXPECT_EQ(task.getState(), ITask::State::Completed);
    event = task.releaseEvent();
    result = task.releaseResult();

    // Log 20 random solutions
    u_int32_t numSolutions = result->hostNumSolutions_;
    for (u_int32_t i = 0; i < 20; ++i)
    {
        u_int32_t solutionIndex = i * numSolutions / 20;
        u_int8_t hitCount = result->hostSolutionHitCounts_[solutionIndex];
        float r = result->hostSolutionRs_[solutionIndex];
        float phi = result->hostSolutionPhis_[solutionIndex];

        std::stringstream ss;
        ss << i << ",\t" << static_cast<unsigned>(hitCount) << ",\t" << r << ",\t" << phi;
        LOG_DEBUG(ss.str());
    }
}

