#pragma once

#include "EventUsm/IWorkerController.h"

#include <gmock/gmock.h>

class IWorkerControllerMock : public IWorkerController
{
public:
    IWorkerControllerMock() = default;
    ~IWorkerControllerMock() override = default;

    MOCK_METHOD(void, onTaskCompleted, (std::unique_ptr<ITask> task), (override));
};