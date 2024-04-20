#include "Logger/Logger.h"
#include "ILoggerMock/ILoggerMock.h"
#include "IQueueMock/IQueueMock.h"
#include "ITaskMock/ITaskMock.h"
#include "IWorkerControllerMock/IWorkerControllerMock.h"
#include "EventUsm/WorkerUsm.h"

#include <gtest/gtest.h>
#include <memory>
#include <vector>

class WorkerUsmTest : public ::testing::Test
{
protected:
    WorkerUsmTest()
    : worker_(queueMock_, workerControllerMock_)
    {
        Logger::ILogger::setGlobalInstance(&loggerMock_);
    }

    ~WorkerUsmTest() override
    {
        Logger::ILogger::setGlobalInstance(nullptr);
    }

    void expectLog(const Logger::LogMessage::Severity severity, const std::string& message)
    {
        EXPECT_CALL(loggerMock_, log(testing::AllOf(
            testing::Property(&Logger::LogMessage::getSeverity, severity),
            testing::Property(&Logger::LogMessage::getMessage, message)
        )));
    }

    void expectTaskSubmittedLog(const ITask::TaskId taskId)
    {
        expectLog(Logger::LogMessage::Severity::Debug, "Task submitted, id: " + std::to_string(taskId));
    }

    Logger::ILoggerMock loggerMock_;
    IWorkerControllerMock workerControllerMock_;
    IQueueMock queueMock_;
    WorkerUsm worker_;
};

TEST_F(WorkerUsmTest, SubmitTask)
{
    std::unique_ptr<ITaskMock> task = std::make_unique<ITaskMock>();
    ITaskMock* taskPtr = task.get();

    constexpr ITask::TaskId taskId = 42;
    EXPECT_CALL(*taskPtr, getState()).WillRepeatedly(testing::Return(ITask::State::EventAssigned));
    EXPECT_CALL(*taskPtr, onAssignedToWorker());
    EXPECT_CALL(*taskPtr, getId()).WillRepeatedly(testing::Return(taskId));
    expectTaskSubmittedLog(taskId);
    EXPECT_TRUE(worker_.submitTask(std::move(task)));

    EXPECT_EQ(1, worker_.getNumberOfTasks());
}

TEST_F(WorkerUsmTest, SubmitNullTask)
{
    expectLog(Logger::LogMessage::Severity::Error, "Attempt to submit nullptr");
    EXPECT_FALSE(worker_.submitTask(nullptr));

    EXPECT_EQ(0, worker_.getNumberOfTasks());
}

TEST_F(WorkerUsmTest, SubmitTaskInvalidState)
{
    std::vector<ITask::State> invalidStates = {
        ITask::State::Created,
        ITask::State::ReadyToQueue,
        ITask::State::WaitingForResources,
        ITask::State::WaitingForEventTransfer,
        ITask::State::WaitingForExecution,
        ITask::State::Executed,
        ITask::State::WaitingForResultTransfer,
        ITask::State::Completed
    };
    for(ITask::State state : invalidStates)
    {
        std::unique_ptr<ITaskMock> task = std::make_unique<ITaskMock>();
        ITaskMock* taskPtr = task.get();

        constexpr ITask::TaskId taskId = 42;
        EXPECT_CALL(*taskPtr, getState()).WillRepeatedly(testing::Return(state));
        EXPECT_CALL(*taskPtr, getId()).WillRepeatedly(testing::Return(taskId));
        expectLog(Logger::LogMessage::Severity::Error, "Attempt to submit task with invalid state, id: " + std::to_string(taskId) + " state: " + ITask::stateToString(state));
        EXPECT_FALSE(worker_.submitTask(std::move(task)));
    }
 
    EXPECT_EQ(0, worker_.getNumberOfTasks());
}

TEST_F(WorkerUsmTest, SubmitMultipleTasksWithSameId)
{
    std::unique_ptr<ITaskMock> task = std::make_unique<ITaskMock>();
    ITaskMock* taskPtr = task.get();

    constexpr ITask::TaskId taskId = 42;
    EXPECT_CALL(*taskPtr, getState()).WillRepeatedly(testing::Return(ITask::State::EventAssigned));
    EXPECT_CALL(*taskPtr, onAssignedToWorker());
    EXPECT_CALL(*taskPtr, getId()).WillRepeatedly(testing::Return(taskId));
    expectTaskSubmittedLog(taskId);
    EXPECT_TRUE(worker_.submitTask(std::move(task)));

    std::unique_ptr<ITaskMock> task2 = std::make_unique<ITaskMock>();
    ITaskMock* taskPtr2 = task2.get();

    EXPECT_CALL(*taskPtr2, getState()).WillRepeatedly(testing::Return(ITask::State::EventAssigned));
    EXPECT_CALL(*taskPtr2, getId()).WillRepeatedly(testing::Return(taskId));
    expectLog(Logger::LogMessage::Severity::Error, "Attempt to submit task with id that already exists, id: " + std::to_string(taskId));
    EXPECT_FALSE(worker_.submitTask(std::move(task2)));
 
    EXPECT_EQ(1, worker_.getNumberOfTasks());
}

class WorkerUsmTakProcessTasksTest : public WorkerUsmTest
{
protected:
    WorkerUsmTakProcessTasksTest()
    {
        std::unique_ptr<ITaskMock> task = std::make_unique<ITaskMock>();
        taskPtr_ = task.get();

        EXPECT_CALL(*taskPtr_, getState()).WillRepeatedly(testing::Return(ITask::State::EventAssigned));
        EXPECT_CALL(*taskPtr_, onAssignedToWorker());
        EXPECT_CALL(*taskPtr_, getId()).WillRepeatedly(testing::Return(taskId_));
        expectTaskSubmittedLog(taskId_);
        
        EXPECT_TRUE(worker_.submitTask(std::move(task)));

        EXPECT_EQ(1, worker_.getNumberOfTasks());
    }

    void expectProcessingTask(ITaskMock* task, ITask::State newState)
    {
        EXPECT_CALL(*task, getState()).WillRepeatedly(testing::Return(newState));
        EXPECT_CALL(*taskPtr_, getId()).WillRepeatedly(testing::Return(taskId_));
        EXPECT_CALL(*task, isStateChanging()).WillRepeatedly(testing::Return(false));
        expectLog(Logger::LogMessage::Severity::Debug, "Processing task after state change, id: " + std::to_string(taskId_) + " new state: " + ITask::stateToString(newState));
    }

    void expectTaskIdLog(const ITask::TaskId taskId)
    {
        expectLog(Logger::LogMessage::Severity::Debug, "Task id: " + std::to_string(taskId));
    }

    static constexpr ITask::TaskId taskId_ = 42;
    ITaskMock* taskPtr_ = nullptr;
};

TEST_F(WorkerUsmTakProcessTasksTest, HandleTaskStateChange)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::EventAssigned);
    expectLog(Logger::LogMessage::Severity::Error, "Task has invalid state, id: " + std::to_string(taskId_) + " state: " + ITask::stateToString(ITask::State::EventAssigned));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processTasksStateChanging)
{
    worker_.onTaskStateChange(*taskPtr_);

    EXPECT_CALL(*taskPtr_, getState()).WillRepeatedly(testing::Return(ITask::State::EventAssigned));
    EXPECT_CALL(*taskPtr_, isStateChanging()).WillRepeatedly(testing::Return(true));
    expectLog(Logger::LogMessage::Severity::Debug, "Processing task after state change, id: " + std::to_string(taskId_) + " new state: " + ITask::stateToString(ITask::State::EventAssigned));
    expectLog(Logger::LogMessage::Severity::Error, "Task state is changing, id: " + std::to_string(taskId_) + " state: " + ITask::stateToString(ITask::State::EventAssigned));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processReadyToQueue)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::ReadyToQueue);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(queueMock_, getWorkCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getWorkLoad()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*taskPtr_, assignQueue(testing::Ref(queueMock_)));
    expectLog(Logger::LogMessage::Severity::Debug, "Task assigned to queue, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processReadyToQueueQueueFull)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::ReadyToQueue);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(queueMock_, getWorkCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getWorkLoad()).WillRepeatedly(testing::Return(1));
    expectLog(Logger::LogMessage::Severity::Debug, "Queue is full");
    worker_.processTasks();

    expectProcessingTask(taskPtr_, ITask::State::ReadyToQueue);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(queueMock_, getWorkCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getWorkLoad()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*taskPtr_, assignQueue(testing::Ref(queueMock_)));
    expectLog(Logger::LogMessage::Severity::Debug, "Task assigned to queue, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processWaitingForResourcesMissingEventResources)
{
    worker_.onTaskStateChange(*taskPtr_);
    EXPECT_CALL(*taskPtr_, isEventResourcesAssigned()).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(*taskPtr_, isResultResourcesAssigned()).WillRepeatedly(testing::Return(true));

    expectProcessingTask(taskPtr_, ITask::State::WaitingForResources);
    expectTaskIdLog(taskId_);
    expectLog(Logger::LogMessage::Severity::Debug, "Waiting for event resources");
    EXPECT_CALL(queueMock_, getEventResourcesCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getEventResourcesLoad()).WillRepeatedly(testing::Return(1));
    expectLog(Logger::LogMessage::Severity::Debug, "Queue has no free event resources");
    worker_.processTasks();

    expectProcessingTask(taskPtr_, ITask::State::WaitingForResources);
    expectTaskIdLog(taskId_);
    expectLog(Logger::LogMessage::Severity::Debug, "Waiting for event resources");
    EXPECT_CALL(queueMock_, getEventResourcesCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getEventResourcesLoad()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*taskPtr_, takeEventResources(testing::_));
    EXPECT_CALL(queueMock_, releaseEventResources()).WillOnce(testing::Return(testing::ByMove(std::make_unique<IQueue::Resources>())));
    expectLog(Logger::LogMessage::Severity::Debug, "Event resources assigned, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processWaitingForResourcesMissingResultResources)
{
    worker_.onTaskStateChange(*taskPtr_);
    EXPECT_CALL(*taskPtr_, isEventResourcesAssigned()).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(*taskPtr_, isResultResourcesAssigned()).WillRepeatedly(testing::Return(false));

    expectProcessingTask(taskPtr_, ITask::State::WaitingForResources);
    expectTaskIdLog(taskId_);
    expectLog(Logger::LogMessage::Severity::Debug, "Waiting for result resources");
    EXPECT_CALL(queueMock_, getResultResourcesCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getResultResourcesLoad()).WillRepeatedly(testing::Return(1));
    expectLog(Logger::LogMessage::Severity::Debug, "Queue has no free result resources");
    worker_.processTasks();

    expectProcessingTask(taskPtr_, ITask::State::WaitingForResources);
    expectTaskIdLog(taskId_);
    expectLog(Logger::LogMessage::Severity::Debug, "Waiting for result resources");
    EXPECT_CALL(queueMock_, getResultResourcesCapacity()).WillRepeatedly(testing::Return(1));
    EXPECT_CALL(queueMock_, getResultResourcesLoad()).WillRepeatedly(testing::Return(0));
    EXPECT_CALL(*taskPtr_, takeResultResources(testing::_));
    EXPECT_CALL(queueMock_, releaseResultResources()).WillOnce(testing::Return(testing::ByMove(std::make_unique<IQueue::Resources>())));
    expectLog(Logger::LogMessage::Severity::Debug, "Result resources assigned, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processWaitingEventTransfer)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::WaitingForEventTransfer);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(*taskPtr_, transferEvent());
    expectLog(Logger::LogMessage::Severity::Debug, "Event transfer started, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processWaitingForExecution)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::WaitingForExecution);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(*taskPtr_, execute());
    expectLog(Logger::LogMessage::Severity::Debug, "Task execution started, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processExecuted)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::Executed);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(queueMock_, takeEventResources(testing::_));
    EXPECT_CALL(*taskPtr_, releaseEventResources()).WillOnce(testing::Return(testing::ByMove(std::make_unique<IQueue::Resources>())));
    expectLog(Logger::LogMessage::Severity::Debug, "Event resources returned to queue, task id: " + std::to_string(taskId_));    
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processWaitingForResultTransfer)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::WaitingForResultTransfer);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(*taskPtr_, transferResult());
    expectLog(Logger::LogMessage::Severity::Debug, "Result transfer started, task id: " + std::to_string(taskId_));
    worker_.processTasks();
}

TEST_F(WorkerUsmTakProcessTasksTest, processCompleted)
{
    worker_.onTaskStateChange(*taskPtr_);

    expectProcessingTask(taskPtr_, ITask::State::Completed);
    expectTaskIdLog(taskId_);
    EXPECT_CALL(queueMock_, takeResultResources(testing::_));
    EXPECT_CALL(*taskPtr_, releaseResultResources()).WillOnce(testing::Return(testing::ByMove(std::make_unique<IQueue::Resources>())));
    expectLog(Logger::LogMessage::Severity::Debug, "Result resources returned to queue, task id: " + std::to_string(taskId_));
    EXPECT_CALL(workerControllerMock_, onTaskCompleted(testing::_)).WillOnce(testing::Invoke([](std::unique_ptr<ITask> task) {
        EXPECT_EQ(taskId_, task->getId());
    }));
    expectLog(Logger::LogMessage::Severity::Debug, "Task completed and will be handed in to controller, task id: " + std::to_string(taskId_));
    worker_.processTasks();

    EXPECT_EQ(0, worker_.getNumberOfTasks());
}