#pragma once

#include "EventUsm/ITask.h"
#include "EventUsm/IWorker.h"

#include <gmock/gmock.h>

class ITaskMock : public ITask
{
public:
    ITaskMock() = default;
    ~ITaskMock() override = default;

    MOCK_METHOD(ITask::TaskId, getId, (), (const, override));
    MOCK_METHOD(State, getState, (), (const, override));
    MOCK_METHOD(bool, isStateChanging, (), (const, override));
    MOCK_METHOD(bool, isResourcesAssigned, (), (const, override));
    MOCK_METHOD(std::chrono::milliseconds, getExecutionTime, (), (const, override));

    MOCK_METHOD(void, takeEventAndResult, ((std::unique_ptr<EventUsm>&& event), (std::unique_ptr<ResultUsm>&& result)), (override));
    MOCK_METHOD(std::unique_ptr<EventUsm>, releaseEvent, (), (override));
    MOCK_METHOD(std::unique_ptr<ResultUsm>, releaseResult, (), (override));

    MOCK_METHOD(void, onAssignedToWorker, (ITaskStateObserver& stateObserver), (override));
    MOCK_METHOD(void, assignQueue, (IQueue& queue), (override));
    MOCK_METHOD(void, takeResources, ((std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&>) resources), (override));
    MOCK_METHOD(void, transferEvent, (), (override));
    MOCK_METHOD(void, execute, (), (override));
    MOCK_METHOD(void, transferResult, (), (override));
    MOCK_METHOD(IQueue::DeviceResourceGroupId, releaseResources, (), (override));

protected:
    MOCK_METHOD(ExecutionEvents, executeOnDevice, (sycl::queue& queue), (override));
};