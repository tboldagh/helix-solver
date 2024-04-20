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

    MOCK_METHOD(void, takeEvent, (std::unique_ptr<EventUsm>&& event), (override));
    MOCK_METHOD(void, releaseEvent, (), (override));
    MOCK_METHOD(void, releaseResult, (), (override));

    MOCK_METHOD(void, onAssignedToWorker, (), (override));
    MOCK_METHOD(void, assignQueue, (IQueue& queue), (override));
    MOCK_METHOD(void, takeEventResources, (std::unique_ptr<IQueue::Resources>&& resources), (override));
    MOCK_METHOD(void, takeResultResources, (std::unique_ptr<IQueue::Resources>&& resources), (override));
    MOCK_METHOD(void, transferEvent, (), (override));
    MOCK_METHOD(void, execute, (), (override));
    MOCK_METHOD(void, transferResult, (), (override));
    MOCK_METHOD(std::unique_ptr<IQueue::Resources>, releaseEventResources, (), (override));
    MOCK_METHOD(std::unique_ptr<IQueue::Resources>, releaseResultResources, (), (override));
};