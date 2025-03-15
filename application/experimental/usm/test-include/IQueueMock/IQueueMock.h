#pragma once

#include "EventUsm/IQueue.h"

#include <gmock/gmock.h>

class IQueueMock : public IQueue
{
public:
    IQueueMock() = default;
    ~IQueueMock() override = default;

    MOCK_METHOD(bool, createResources, (const CreateResourceGroupFunction& createResourceGroupFunction), (override));

    MOCK_METHOD(Capacity, getResourcesCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getResourcesLoad, (), (const, override));
    MOCK_METHOD((std::pair<DeviceResourceGroupId, const DeviceResourceGroup&>), getResources, (), (override));
    MOCK_METHOD(void, returnResources, (DeviceResourceGroupId resourceGroupId), (override));

    MOCK_METHOD(Capacity, getWorkCapacity, (), (const, override));
    MOCK_METHOD(Capacity, getWorkLoad, (), (const, override));
    MOCK_METHOD(void, incrementWorkLoad, (), (override));
    MOCK_METHOD(void, decrementWorkLoad, (), (override));

    MOCK_METHOD(sycl::queue&, checkoutQueue, (), (override));
    MOCK_METHOD(void, checkinQueue, (), (override));
    MOCK_METHOD(const sycl::queue&, getQueue, (), (const, override));
};