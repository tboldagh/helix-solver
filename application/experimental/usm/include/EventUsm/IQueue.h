#pragma once

#include "EventUsm/DeviceResource.h"

#include <CL/sycl.hpp>
#include <map>
#include <memory>


class IQueue
{
public:
    using Capacity = u_int16_t;
    using DeviceResourceGroupId = u_int16_t;

    virtual ~IQueue() = default;

    virtual Capacity getEventResourcesCapacity() const = 0;
    virtual Capacity getEventResourcesLoad() const = 0;
    // Used to borrow a resource group from the queue. Resource group has to be returned with returnEventResourceGroup.
    virtual std::pair<DeviceResourceGroupId, const DeviceResourceGroup&> getEventResourceGroup() = 0;
    virtual void returnEventResourceGroup(DeviceResourceGroupId resourceGroupId) = 0;

    virtual Capacity getResultResourcesCapacity() const = 0;
    virtual Capacity getResultResourcesLoad() const = 0;
    virtual std::pair<DeviceResourceGroupId, const DeviceResourceGroup&> getResultResourceGroup() = 0;
    virtual void returnResultResourceGroup(DeviceResourceGroupId resourceGroupId) = 0;

    virtual Capacity getWorkCapacity() const = 0;
    virtual Capacity getWorkLoad() const = 0;
    virtual void incrementWorkLoad() = 0;
    virtual void decrementWorkLoad() = 0;

    virtual sycl::queue& checkoutQueue() = 0;
    virtual void checkinQueue() = 0;
    // Used to get queue when no lock is needed.
    virtual const sycl::queue& getQueue() const = 0;
};