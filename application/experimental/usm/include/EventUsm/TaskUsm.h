#pragma once

#include "EventUsm/ITask.h"

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

    void takeEvent(std::unique_ptr<EventUsm>&& event) override;
    void onAssignedToWorker(IStateObserver& stateObserver) override;
    void assignQueue(IQueue& queue) override;
    void takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources) override;
    void takeResultResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resultResources) override;
    void transferEvent() override;
    void execute() override;
    void transferResult() override;
    IQueue::DeviceResourceGroupId releaseEventResourceGroup() override;
    IQueue::DeviceResourceGroupId releaseResultResourceGroup() override;

private:
    void setState(State state);
    void checkResourcesAssigned();
    void transferEventToDeviceThread();
    void executeThread();
    void transferResultFromDeviceThread();

    const ITask::TaskId id_;

    State state_ = State::Created;
    bool isStateChanging_ = false;
    ITaskStateObserver* stateObserver_ = nullptr;

    std::unique_ptr<EventUsm> event_;
    std::unique_ptr<ResultUsm> result_;
    IQueue* queue_;
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