#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"
#include "IQueueMock/IQueueMock.h"
#include "ITaskStateObserverMock/ITaskStateObserverMock.h"
#include "EventUsm/TaskUsm.h"

#include <sycl/sycl.hpp>
#include <gtest/gtest.h>


class TestTask : public TaskUsm
{
public:
    explicit TestTask(ITask::TaskId id) : TaskUsm(id) {}
    ~TestTask() override = default;

    bool isExecuted() const
    {
        return isExecuted_;
    }

protected:
    ExecutionEvents executeOnDevice(sycl::queue& syclQueue) override
    {
        isExecuted_ = true;
        return ExecutionEvents{};
    }

private:
    bool isExecuted_ = false;
};

class TaskUsmTest : public ::testing::Test
{
protected:
    TaskUsmTest()
    {
        Logger::ILogger::setGlobalInstance(&loggerMock_);
    }

    ~TaskUsmTest() override
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

    static constexpr ITask::TaskId taskId_ = 42;
    TestTask task_{taskId_};
    ITaskStateObserverMock stateObserverMock_;
    sycl::queue syclQueue_{sycl::gpu_selector_v};
    IQueueMock queueMock_;
};

TEST_F(TaskUsmTest, InitialState)
{
    ASSERT_EQ(task_.getId(), taskId_);
    ASSERT_EQ(task_.getState(), ITask::State::Created);
    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_FALSE(task_.isResourcesAssigned());
}

TEST_F(TaskUsmTest, TakeEventAndResult)
{
    ASSERT_EQ(task_.getState(), ITask::State::Created);

    constexpr EventUsm::EventId eventId = 21;
    constexpr ResultUsm::ResultId resultId = 37;
    task_.takeEventAndResult(std::make_unique<EventUsm>(eventId), std::make_unique<ResultUsm>(resultId));

    ASSERT_EQ(task_.getState(), ITask::State::EventAndResultAssigned);
}

TEST_F(TaskUsmTest, TakeEventAndResultTwice)
{
    ASSERT_EQ(task_.getState(), ITask::State::Created);

    constexpr EventUsm::EventId eventIdA = 21;
    constexpr ResultUsm::ResultId resultIdA = 37;
    task_.takeEventAndResult(std::make_unique<EventUsm>(eventIdA), std::make_unique<ResultUsm>(resultIdA));

    ASSERT_EQ(task_.getState(), ITask::State::EventAndResultAssigned);

    constexpr EventUsm::EventId eventIdB = 22;
    constexpr ResultUsm::ResultId resultIdB = 38;
    expectLog(Logger::LogMessage::Severity::Warning, "Task already has an event assiged but it's taking a new one, task id: " + std::to_string(taskId_) + ", event id: " + std::to_string(eventIdA) + ", new event id: " + std::to_string(eventIdB));
    task_.takeEventAndResult(std::make_unique<EventUsm>(eventIdB), std::make_unique<ResultUsm>(resultIdB));
}

TEST_F(TaskUsmTest, ReleaseEvent)
{
    ASSERT_EQ(task_.getState(), ITask::State::Created);

    constexpr EventUsm::EventId eventId = 21;
    task_.takeEventAndResult(std::make_unique<EventUsm>(eventId), std::make_unique<ResultUsm>(37));

    ASSERT_EQ(task_.getState(), ITask::State::EventAndResultAssigned);

    auto event = task_.releaseEvent();
    ASSERT_EQ(event->eventId_, eventId);
    ASSERT_FALSE(task_.event_);
}

TEST_F(TaskUsmTest, ReleaseResult)
{
    ASSERT_EQ(task_.getState(), ITask::State::Created);

    constexpr ResultUsm::ResultId resultId = 37;
    task_.takeEventAndResult(std::make_unique<EventUsm>(21), std::make_unique<ResultUsm>(resultId));

    ASSERT_EQ(task_.getState(), ITask::State::EventAndResultAssigned);

    auto result = task_.releaseResult();
    ASSERT_EQ(result->resultId_, resultId);
    ASSERT_FALSE(task_.result_);
}

TEST_F(TaskUsmTest, OnAssignedToWorker)
{
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.onAssignedToWorker(stateObserverMock_);

    ASSERT_EQ(task_.getState(), ITask::State::ReadyToQueue);
}

TEST_F(TaskUsmTest, AssignQueue)
{
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.onAssignedToWorker(stateObserverMock_);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.assignQueue(queueMock_);
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);
}

class TaskUsmResourcesTest : public TaskUsmTest
{
protected:
    TaskUsmResourcesTest()
    : event_(std::make_unique<EventUsm>(eventId_))
    , result_(std::make_unique<ResultUsm>(resultId_))
    , eventPtr_(event_.get())
    , resultPtr_(result_.get())
    {
        task_.takeEventAndResult(std::move(event_), std::move(result_));

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.onAssignedToWorker(stateObserverMock_);

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        EXPECT_CALL(queueMock_, getQueue()).WillRepeatedly(testing::ReturnRef(syclQueue_));
        task_.assignQueue(queueMock_);

        eventKernelMemory_.allocate();
        resultKernelMemory_.allocate();
        resources_ = DeviceResourceGroup{
            {DeviceResourceType::EventKernelMemory, &eventKernelMemory_},
            {DeviceResourceType::ResultKernelMemory, &resultKernelMemory_}
        };
    }

    ~TaskUsmResourcesTest() override
    {
        eventKernelMemory_.deallocate();
        resultKernelMemory_.deallocate();
    }

    static constexpr EventUsm::EventId eventId_ = 21;
    static constexpr ResultUsm::ResultId resultId_ = 37;
    static constexpr ITask::TaskId taskId_ = 42;
    static constexpr IQueue::DeviceResourceGroupId resourceGroupId_ = 43;

    EventUsm::EventKernelMemory eventKernelMemory_{syclQueue_};
    ResultUsm::ResultKernelMemory resultKernelMemory_{syclQueue_};
    DeviceResourceGroup resources_;
    std::unique_ptr<EventUsm> event_;
    std::unique_ptr<ResultUsm> result_;
    EventUsm* eventPtr_;
    ResultUsm* resultPtr_;
};

TEST_F(TaskUsmResourcesTest, TakeResources)
{
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.takeResources(std::make_pair(resourceGroupId_, resources_));
    ASSERT_TRUE(task_.isResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForEventTransfer);
    ASSERT_TRUE(task_.event_->isKernelMemorySet());
    ASSERT_EQ(task_.event_->kernelMemory_, &eventKernelMemory_);
    ASSERT_TRUE(task_.event_->kernelMemory_->isAllocated());
    ASSERT_TRUE(task_.result_->isKernelMemorySet());
    ASSERT_EQ(task_.result_->kernelMemory_, &resultKernelMemory_);
    ASSERT_TRUE(task_.result_->kernelMemory_->isAllocated());
}

TEST_F(TaskUsmResourcesTest, TransferEvent)
{
    // Test only the thread function instead of transferEvent() method because it spawns a thread

    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.takeResources(std::make_pair(resourceGroupId_, resources_));
    ASSERT_TRUE(task_.isResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForEventTransfer);

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.isStateChanging_ = true;
    task_.transferEventToDeviceThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForExecution);
}

class TaskUsmExecutionTest : public TaskUsmResourcesTest
{
protected:
    TaskUsmExecutionTest()
    {
        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.takeResources(std::make_pair(resourceGroupId_, resources_));

        EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
        EXPECT_CALL(queueMock_, checkinQueue());
        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        task_.isStateChanging_ = true;
        task_.transferEventToDeviceThread();
    }

    ~TaskUsmExecutionTest() override = default;
};

TEST_F(TaskUsmExecutionTest, Execute)
{
    // Test only the thread function instead of execute() method because it spawns a thread

    ASSERT_EQ(task_.getState(), ITask::State::WaitingForExecution);

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.isStateChanging_ = true;
    task_.executeThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::Executed);
    ASSERT_TRUE(task_.isExecuted());
}

TEST_F(TaskUsmExecutionTest, TransferResult)
{
    // Test only the thread function instead of transferResult() method because it spawns a thread

    ASSERT_EQ(task_.getState(), ITask::State::WaitingForExecution);

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.isStateChanging_ = true;
    task_.executeThread();

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.isStateChanging_ = true;
    task_.transferResultFromDeviceThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::ResultTransferred);
}

TEST_F(TaskUsmExecutionTest, ReleaseResources)
{
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForExecution);

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.isStateChanging_ = true;
    task_.executeThread();

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.isStateChanging_ = true;
    task_.transferResultFromDeviceThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::ResultTransferred);

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    IQueue::DeviceResourceGroupId resourceGroupId = task_.releaseResources();
    ASSERT_FALSE(task_.isResourcesAssigned());
    ASSERT_EQ(resourceGroupId, resourceGroupId_);
    ASSERT_EQ(task_.getState(), ITask::State::Completed);
}
