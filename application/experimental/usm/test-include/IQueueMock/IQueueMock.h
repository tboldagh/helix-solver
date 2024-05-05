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
    MOCK_METHOD((std::pair<DeviceResourceGroupId, const DeviceResourceGroup&>), getEventResourceGroup, (), (override));
    MOCK_METHOD(void, returnEventResourceGroup, (DeviceResourceGroupId resourceGroupId), (override));

    MOCK_METHOD(Capacity, getResultResourcesCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getResultResourcesLoad, (), (const, override));
    MOCK_METHOD((std::pair<DeviceResourceGroupId, const DeviceResourceGroup&>), getResultResourceGroup, (), (override));
    MOCK_METHOD(void, returnResultResourceGroup, (DeviceResourceGroupId resourceGroupId), (override));

    MOCK_METHOD(Capacity, getWorkCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getWorkLoad, (), (const, override));
    MOCK_METHOD(void, incrementWorkLoad, (), (override));
    MOCK_METHOD(void, decrementWorkLoad, (), (override));

    MOCK_METHOD(sycl::queue&, checkoutQueue, (), (override));
    MOCK_METHOD(void, checkinQueue, (), (override));
    MOCK_METHOD(const sycl::queue&, getQueue, (), (const, override));
};