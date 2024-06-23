#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"
#include "IQueueMock/IQueueMock.h"
#include "ITaskStateObserverMock/ITaskStateObserverMock.h"
#include "EventUsm/TaskUsm.h"

#include <CL/sycl.hpp>
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
        return ExecutionEvents();
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

    ~TaskUsmTest()
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
    ASSERT_FALSE(task_.isEventResourcesAssigned());
    ASSERT_FALSE(task_.isResultResourcesAssigned());
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
    expectLog(Logger::LogMessage::Severity::Warning, "Task already has a result assiged but it's taking a new one, task id: " + std::to_string(taskId_) + ", result id: " + std::to_string(resultIdA) + ", new result id: " + std::to_string(resultIdB));
    task_.takeEventAndResult(std::make_unique<EventUsm>(eventIdB), std::make_unique<ResultUsm>(resultIdB));
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

class TaskUsmTakeResourcesTest : public TaskUsmTest
{
protected:
    TaskUsmTakeResourcesTest()
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
    }

    ~TaskUsmTakeResourcesTest() = default;

    static constexpr EventUsm::EventId eventId_ = 21;
    static constexpr ResultUsm::ResultId resultId_ = 37;
    static constexpr ITask::TaskId taskId_ = 42;
    static constexpr IQueue::DeviceResourceGroupId eventResourceGroupId_ = 43;
    static constexpr IQueue::DeviceResourceGroupId resultResourceGroupId_ = 44;

    std::unique_ptr<EventUsm> event_;
    std::unique_ptr<ResultUsm> result_;
    EventUsm* eventPtr_;
    ResultUsm* resultPtr_;
};

TEST_F(TaskUsmTakeResourcesTest, TakeEventResources)
{
    const std::unique_ptr<DeviceResourceGroup> eventResources = EventUsm::allocateDeviceResources(syclQueue_);
    task_.takeEventResources(std::make_pair(eventResourceGroupId_, *eventResources));

    ASSERT_EQ(eventPtr_->deviceNumPoints_, eventResources->at(DeviceResourceType::NumPoints));
    ASSERT_EQ(eventPtr_->deviceXs_, eventResources->at(DeviceResourceType::Xs));
    ASSERT_EQ(eventPtr_->deviceYs_, eventResources->at(DeviceResourceType::Ys));
    ASSERT_EQ(eventPtr_->deviceZs_, eventResources->at(DeviceResourceType::Zs));
    ASSERT_EQ(eventPtr_->deviceLayers_, eventResources->at(DeviceResourceType::Layers));

    // Event borrowed resources so it should not deallocate them
    EventUsm::deallocateDeviceResources(*eventResources, syclQueue_);

    ASSERT_TRUE(task_.isEventResourcesAssigned());
    ASSERT_FALSE(task_.isResultResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);
}

TEST_F(TaskUsmTakeResourcesTest, TakeResultResources)
{
    const std::unique_ptr<DeviceResourceGroup> resultResources = ResultUsm::allocateDeviceResources(syclQueue_);
    task_.takeResultResources(std::make_pair(resultResourceGroupId_, *resultResources));

    ASSERT_EQ(resultPtr_->deviceNumSolutions_, resultResources->at(DeviceResourceType::NumSolutions));
    ASSERT_EQ(resultPtr_->deviceSomeSolutionParameters_, resultResources->at(DeviceResourceType::SomeSolutionParameters));

    // Result borrowed resources so it should not deallocate them
    ResultUsm::deallocateDeviceResources(*resultResources, syclQueue_);

    ASSERT_FALSE(task_.isEventResourcesAssigned());
    ASSERT_TRUE(task_.isResultResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForResources);
}

TEST_F(TaskUsmTakeResourcesTest, TakeEventResourcesAndRestultResources)
{
    const std::unique_ptr<DeviceResourceGroup> eventResources = EventUsm::allocateDeviceResources(syclQueue_);
    task_.takeEventResources(std::make_pair(eventResourceGroupId_, *eventResources));

    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    const std::unique_ptr<DeviceResourceGroup> resultResources = ResultUsm::allocateDeviceResources(syclQueue_);
    task_.takeResultResources(std::make_pair(resultResourceGroupId_, *resultResources));

    // Event borrowed resources so it should not deallocate them
    EventUsm::deallocateDeviceResources(*eventResources, syclQueue_);

    // Result borrowed resources so it should not deallocate them
    ResultUsm::deallocateDeviceResources(*resultResources, syclQueue_);

    ASSERT_TRUE(task_.isEventResourcesAssigned());
    ASSERT_TRUE(task_.isResultResourcesAssigned());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForEventTransfer);
}

class TaskUsmExecutionTest : public TaskUsmTakeResourcesTest
{
protected:
    TaskUsmExecutionTest()
    {
        eventResources_ = EventUsm::allocateDeviceResources(syclQueue_);
        task_.takeEventResources(std::make_pair(eventResourceGroupId_, *eventResources_));

        EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
        resultResources_ = ResultUsm::allocateDeviceResources(syclQueue_);
        task_.takeResultResources(std::make_pair(resultResourceGroupId_, *resultResources_));
    }

    ~TaskUsmExecutionTest()
    {
        // Resources borrowed by the tast so it should not deallocate them
        EventUsm::deallocateDeviceResources(*eventResources_, syclQueue_);
        ResultUsm::deallocateDeviceResources(*resultResources_, syclQueue_);
    }

    static constexpr IQueue::DeviceResourceGroupId eventResourceGroupId_ = 43;
    static constexpr IQueue::DeviceResourceGroupId resultResourceGroupId_ = 44;

    std::unique_ptr<DeviceResourceGroup> eventResources_;
    std::unique_ptr<DeviceResourceGroup> resultResources_;
};

TEST_F(TaskUsmExecutionTest, TransferEventThread)
{
    // We test only the thread function instead of transferEvent() method because it spawns a thread

    task_.isStateChanging_ = true;

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.transferEventToDeviceThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::WaitingForExecution);
}

TEST_F(TaskUsmExecutionTest, ExecuteThread)
{
    // We test only the thread function instead of execute() method because it spawns a thread

    task_.isStateChanging_ = true;

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.executeThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::Executed);
    ASSERT_TRUE(task_.isExecuted());
}

TEST_F(TaskUsmExecutionTest, TransferResultThread)
{
    // We test only the thread function instead of transferResult() method because it spawns a thread

    task_.isStateChanging_ = true;

    EXPECT_CALL(queueMock_, checkoutQueue()).WillOnce(testing::ReturnRef(syclQueue_));
    EXPECT_CALL(queueMock_, checkinQueue());
    EXPECT_CALL(stateObserverMock_, onTaskStateChange(testing::Ref(task_)));
    task_.transferResultFromDeviceThread();

    ASSERT_FALSE(task_.isStateChanging());
    ASSERT_EQ(task_.getState(), ITask::State::Completed);
}