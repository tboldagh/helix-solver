#pragma once

#include "EventUsm/ITask.h"

#include <gtest/gtest.h>


class ITaskStateObserver;

class TaskUsm : public ITask
{
public:
    explicit TaskUsm(ITask::TaskId id);
    ~TaskUsm() override;

    inline ITask::TaskId getId() const override;
    inline State getState() const override;
    inline bool isStateChanging() const override;
    inline bool isResourcesAssigned() const override;
    inline std::chrono::milliseconds getExecutionTime() const override;

    void takeEventAndResult(std::unique_ptr<EventUsm>&& event, std::unique_ptr<ResultUsm>&& result) override;
    virtual std::unique_ptr<EventUsm> releaseEvent() override;
    virtual std::unique_ptr<ResultUsm> releaseResult() override;

    void onAssignedToWorker(ITaskStateObserver& stateObserver) override;
    void assignQueue(IQueue& queue) override;
    void takeResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resources) override;
    void transferEvent() override;
    void execute() override;
    void transferResult() override;
    IQueue::DeviceResourceGroupId releaseResources() override;

protected:
    void setState(State state);
    
    std::unique_ptr<EventUsm> event_;
    std::unique_ptr<ResultUsm> result_;
    IQueue* queue_;
    State state_ = State::Created;  // Only one state change allowed between calls to onTaskStateChange on tx_
    bool isStateChanging_ = false;
    const ITask::TaskId id_;

private:
    void transferEventToDeviceThread();
    void executeThread();
    void transferResultFromDeviceThread();

    ITaskStateObserver* stateObserver_ = nullptr;
    bool resourcesAssigned_ = false;
    IQueue::DeviceResourceGroupId resourceGroupId_;

    std::chrono::steady_clock::time_point executionStart_;
    std::chrono::steady_clock::time_point executionEnd_;

    FRIEND_TEST(TaskUsmTest, ReleaseEvent);
    FRIEND_TEST(TaskUsmTest, ReleaseResult);
    FRIEND_TEST(TaskUsmResourcesTest, TakeResources);
    FRIEND_TEST(TaskUsmResourcesTest, TransferEvent);
    friend class TaskUsmExecutionTest;
    FRIEND_TEST(TaskUsmExecutionTest, Execute);
    FRIEND_TEST(TaskUsmExecutionTest, TransferResult);
    FRIEND_TEST(TaskUsmExecutionTest, ReleaseResources);
    FRIEND_TEST(HelixSolverTaskFullEvent, Basic);
    friend class HelixSolverTaskExecutionTest;
    FRIEND_TEST(HelixSolverTaskExecutionTest, ExecuteOnDevice);
};

inline ITask::TaskId TaskUsm::getId() const
{
    return id_;
}

inline ITask::State TaskUsm::getState() const
{
    return state_;
}

inline bool TaskUsm::isStateChanging() const
{
    return isStateChanging_;
}

inline bool TaskUsm::isResourcesAssigned() const
{
    return resourcesAssigned_;
}

inline std::chrono::milliseconds TaskUsm::getExecutionTime() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(executionEnd_ - executionStart_);
}
