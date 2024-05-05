#pragma once

#include "EventUsm/IQueue.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"

#include <map>
#include <memory>
#include <string>


class ITaskStateObserver;


class ITask
{
public:
    using TaskId = u_int32_t;
    using ExecutionEvents = std::unordered_set<std::unique_ptr<sycl::event>>;
    
    enum class State
    {
        Created,    // Created, no event assigned
        EventAssigned, // Created, event assigned, not submitted to worker
        ReadyToQueue, // Submitted to worker, worker took ownership, not queued
        WaitingForResources, // Queued, some resources are missing
        WaitingForEventTransfer, // Queued, resources assigned
        WaitingForExecution, // Resources transferred to device
        Executed, // Execution finished, event resources can be released
        WaitingForResultTransfer, // Execution finished, event resources released
        Completed // Execution finished, event resources released, solutions transferred to host, worker can release ownership
    };
    static std::string stateToString(State state);

    virtual ~ITask() = default;

    virtual TaskId getId() const = 0;
    virtual State getState() const = 0;
    // Indicates whether state changing operation is in progress. If true, task should not be manipulated.
    virtual bool isStateChanging() const = 0;
    virtual bool isEventResourcesAssigned() const = 0;
    virtual bool isResultResourcesAssigned() const = 0;

    virtual void takeEvent(std::unique_ptr<EventUsm>&& event) = 0;
    // TODO
    // virtual void releaseEvent(/* TODO */) = 0;
    // virtual void releaseResult(/* TODO */) = 0;

    virtual void onAssignedToWorker(ITaskStateObserver& stateObserver) = 0;
    virtual void assignQueue(IQueue& queue) = 0;
    virtual void takeEventResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> eventResources) = 0;
    virtual void takeResultResources(std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> resultResources) = 0;
    virtual void transferEvent() = 0;
    virtual void execute() = 0;
    virtual void transferResult() = 0;
    virtual IQueue::DeviceResourceGroupId releaseEventResourceGroup() = 0;
    virtual IQueue::DeviceResourceGroupId releaseResultResourceGroup() = 0;

protected:
    // The actual job to be executed on the device. Will be called from separate thread.
    // Returns sycl::event set. Each events needs to be waited on before the task is considered finished.
    virtual ExecutionEvents executeOnDevice(sycl::queue& syclQueue) = 0;
};