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
    MOCK_METHOD(bool, isEventResourcesAssigned, (), (const, override));
    MOCK_METHOD(bool, isResultResourcesAssigned, (), (const, override));
    MOCK_METHOD(std::chrono::milliseconds, getExecutionTime, (), (const, override));

    MOCK_METHOD(void, takeEventAndResult, ((std::unique_ptr<EventUsm>&& event), (std::unique_ptr<ResultUsm>&& result)), (override));
    // MOCK_METHOD(void, releaseEvent, (), (override));
    // MOCK_METHOD(void, releaseResult, (), (override));

    MOCK_METHOD(void, onAssignedToWorker, (ITaskStateObserver& stateObserver), (override));
    MOCK_METHOD(void, assignQueue, (IQueue& queue), (override));
    MOCK_METHOD(void, takeEventResources, ((std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&>) eventResources), (override));
    MOCK_METHOD(void, takeResultResources, ((std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&>) resultResources), (override));
    MOCK_METHOD(void, transferEvent, (), (override));
    MOCK_METHOD(void, execute, (), (override));
    MOCK_METHOD(void, transferResult, (), (override));
    MOCK_METHOD(IQueue::DeviceResourceGroupId, releaseEventResourceGroup, (), (override));
    MOCK_METHOD(IQueue::DeviceResourceGroupId, releaseResultResourceGroup, (), (override));

protected:
    MOCK_METHOD(ExecutionEvents, executeOnDevice, (sycl::queue& queue), (override));
};