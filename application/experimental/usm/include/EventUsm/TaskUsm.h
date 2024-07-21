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
    inline bool isEventResourcesAssigned() const override;
    inline bool isResultResourcesAssigned() const override;

    void takeEventAndResult(std::unique_ptr<EventUsm>&& event, std::unique_ptr<ResultUsm>&& result) override;
    void onAssignedToWorker(ITaskStateObserver& stateObserver) override;
    void assignQueue(IQueue& queue) override;
    void takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources) override;
    void takeResultResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resultResources) override;
    void transferEvent() override;
    void execute() override;
    void transferResult() override;
    IQueue::DeviceResourceGroupId releaseEventResourceGroup() override;
    IQueue::DeviceResourceGroupId releaseResultResourceGroup() override;

protected:
    std::unique_ptr<EventUsm> event_;
    std::unique_ptr<ResultUsm> result_;
    IQueue* queue_;

private:
    FRIEND_TEST(TaskUsmExecutionTest, TransferEventThread);
    FRIEND_TEST(TaskUsmExecutionTest, ExecuteThread);
    FRIEND_TEST(TaskUsmExecutionTest, TransferResultThread);

    void setState(State state);
    void checkResourcesAssigned();
    void transferEventToDeviceThread();
    void executeThread();
    void transferResultFromDeviceThread();

    const ITask::TaskId id_;

    State state_ = State::Created;  // Only one state change allowed between calls to onTaskStateChange on tx_
    bool isStateChanging_ = false;

    ITaskStateObserver* stateObserver_ = nullptr;
    bool eventResourcesAssigned_ = false;
    bool resultResourcesAssigned_ = false;
    IQueue::DeviceResourceGroupId eventResourceGroupId_;
    IQueue::DeviceResourceGroupId resultResourceGroupId_;
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

inline bool TaskUsm::isEventResourcesAssigned() const
{
    return eventResourcesAssigned_;
}

inline bool TaskUsm::isResultResourcesAssigned() const
{
    return resultResourcesAssigned_;
}