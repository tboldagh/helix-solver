#pragma once

#include "EventUsm/IQueue.h"

#include <gmock/gmock.h>

class IQueueMock : public IQueue
{
public:
    IQueueMock() = default;
    ~IQueueMock() override = default;

    MOCK_METHOD(Capacity, getEventResourcesCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getEventResourcesLoad, (), (const, override));
    MOCK_METHOD(std::unique_ptr<IQueue::Resources>, releaseEventResources, (), (const, override));
    MOCK_METHOD(void, takeEventResources, (std::unique_ptr<IQueue::Resources>&& resources), (override));

    MOCK_METHOD(Capacity, getResultResourcesCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getResultResourcesLoad, (), (const, override));
    MOCK_METHOD(std::unique_ptr<IQueue::Resources>, releaseResultResources, (), (const, override));
    MOCK_METHOD(void, takeResultResources, (std::unique_ptr<IQueue::Resources>&& resources), (override));

    MOCK_METHOD(Capacity, getWorkCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getWorkLoad, (), (const, override));
};