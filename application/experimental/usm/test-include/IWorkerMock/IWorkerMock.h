#pragma once

#include "EventUsm/IWorker.h"

#include <gmock/gmock.h>

class IWorkerMock : public IWorker
{
public:
    IWorkerMock() = default;
    ~IWorkerMock() override = default;

    MOCK_METHOD(bool, submitTask, (std::unique_ptr<ITask> task), (override));
    MOCK_METHOD(u_int16_t, getNumberOfTasks, (), (const, override));
    MOCK_METHOD(void, processTasks, (), (override));
    MOCK_METHOD(void, onTaskUpdated, (ITask* task), (override));
};