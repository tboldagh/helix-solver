#pragma once

#include "EventUsm/ITaskStateObserver.h"

#include <gmock/gmock.h>

class ITaskStateObserverMock : public ITaskStateObserver
{
public:
    ITaskStateObserverMock() = default;
    ~ITaskStateObserverMock() override = default;

    MOCK_METHOD(void, onTaskStateChange, (ITask& task), (override));
};