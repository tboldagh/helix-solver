#include "HelixSolverUsm/HelixSolverTask.h"
#include "HelixSolverUsm/SingleRegionKernel.h"
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

        deviceResources_ = SingleRegionKernel::createResourceGroup(syclQueue_);
    }

    ~HelixSolverTaskInitTest() override
    {
        for (auto& [type, resource] : *deviceResources_)
        {
            static_cast<KernelMemory*>(resource)->deallocate();
        }

        Logger::ILogger::setGlobalInstance(nullptr);
    }

    void prepareResources()
    {
        splitter_.setKernelMemory(static_cast<KernelMemory*>(deviceResources_->at(DeviceResourceType::SplitterSettingsKernelMemory)));
        splitter_.transferToDevice();
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

    std::unique_ptr<DeviceResourceGroup> deviceResources_;
};

TEST_F(HelixSolverTaskInitTest, TakeResources)
{
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.takeResources(std::make_pair(eventResourceGroupId_, *deviceResources_));
    ASSERT_TRUE(task_.isResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForEventTransfer);
    ASSERT_TRUE(task_.event_->isKernelMemorySet());
    ASSERT_EQ(eventPtr_->kernelMemory_, deviceResources_->at(DeviceResourceType::EventKernelMemory));
    ASSERT_TRUE(eventPtr_->kernelMemory_->isAllocated());
    ASSERT_TRUE(task_.result_->isKernelMemorySet());
    ASSERT_EQ(resultPtr_->kernelMemory_, deviceResources_->at(DeviceResourceType::ResultKernelMemory));
    ASSERT_TRUE(resultPtr_->kernelMemory_->isAllocated());
    ASSERT_EQ(task_.singleRegionKernelMemory_, deviceResources_->at(DeviceResourceType::KernelMemory));
    ASSERT_TRUE(task_.singleRegionKernelMemory_->isAllocated());
    ASSERT_EQ(task_.splitter_.splitterSettingsKernelMemory_, deviceResources_->at(DeviceResourceType::SplitterSettingsKernelMemory));
    ASSERT_TRUE(task_.splitter_.splitterSettingsKernelMemory_->isAllocated());
}

class HelixSolverTaskExecutionTest : public HelixSolverTaskInitTest
{
protected:
    HelixSolverTaskExecutionTest()
    : HelixSolverTaskInitTest()
    {
        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.takeResources(std::make_pair(eventResourceGroupId_, *deviceResources_));
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

    task_.isStateChanging_ = true;
    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.transferResultFromDeviceThread();
    EXPECT_EQ(task_.getState(), ITask::State::ResultTransferred);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    ASSERT_EQ(task_.releaseResources(), eventResourceGroupId_);
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

    constexpr IQueue::DeviceResourceGroupId resourceGroupId = 43;
    std::unique_ptr<DeviceResourceGroup> deviceResources = SingleRegionKernel::createResourceGroup(syclQueue);
    splitter_.setKernelMemory(static_cast<KernelMemory*>(deviceResources->at(DeviceResourceType::SplitterSettingsKernelMemory)));
    splitter_.transferToDevice();

    task.takeEventAndResult(std::move(event), std::move(result));

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.onAssignedToWorker(stateObserverMock);
    EXPECT_EQ(task.getState(), ITask::State::ReadyToQueue);

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    EXPECT_CALL(queueMock, getQueue()).WillRepeatedly(testing::ReturnRef(syclQueue));
    task.assignQueue(queueMock);
    EXPECT_EQ(task.getState(), ITask::State::WaitingForResources);

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.takeResources(std::make_pair(resourceGroupId, *deviceResources));
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

    task.isStateChanging_ = true;
    EXPECT_CALL(queueMock, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue));
    EXPECT_CALL(queueMock, checkinQueue());
    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.transferResultFromDeviceThread();
    EXPECT_EQ(task.getState(), ITask::State::ResultTransferred);

    EXPECT_CALL(stateObserverMock, onTaskStateChange(testing::Ref(task)));
    task.releaseResources();
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

