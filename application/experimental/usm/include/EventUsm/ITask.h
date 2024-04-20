#pragma once

#include "IQueue.h"
#include "EventUsm.h"

#include <map>
#include <memory>
#include <string>

class ITask
{
public:
    using TaskId = u_int32_t;
    
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
    virtual bool isStateChanging() const = 0;
    virtual bool isEventResourcesAssigned() const = 0;
    virtual bool isResultResourcesAssigned() const = 0;

    virtual void takeEvent(std::unique_ptr<EventUsm>&& event) = 0;
    virtual void releaseEvent() = 0;
    virtual void releaseResult() = 0;

    virtual void onAssignedToWorker() = 0;
    virtual void assignQueue(IQueue& queue) = 0;
    virtual void takeEventResources(std::unique_ptr<IQueue::Resources>&& resources) = 0;
    virtual void takeResultResources(std::unique_ptr<IQueue::Resources>&& resources) = 0;
    virtual void transferEvent() = 0;
    virtual void execute() = 0;
    virtual void transferResult() = 0;
    virtual std::unique_ptr<IQueue::Resources> releaseEventResources() = 0;
    virtual std::unique_ptr<IQueue::Resources> releaseResultResources() = 0;
};