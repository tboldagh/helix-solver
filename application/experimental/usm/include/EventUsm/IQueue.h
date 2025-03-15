#pragma once

#include "EventUsm/DeviceResource.h"

#include <sycl/sycl.hpp>
#include <map>
#include <memory>
#include <functional>


class IQueue
{
public:
    using Capacity = u_int16_t;
    using DeviceResourceGroupId = u_int16_t;
    using CreateResourceGroupFunction = std::function<std::unique_ptr<DeviceResourceGroup>(sycl::queue&)>;

    virtual ~IQueue() = default;

    virtual bool createResources(const CreateResourceGroupFunction& createResourceGroupFunction) = 0;

    virtual Capacity getResourcesCapacity() const = 0;
    virtual Capacity getResourcesLoad() const = 0;
    virtual std::pair<DeviceResourceGroupId, const DeviceResourceGroup&> getResources() = 0;
    virtual void returnResources(DeviceResourceGroupId resourceGroupId) = 0;

    virtual Capacity getWorkCapacity() const = 0;
    virtual Capacity getWorkLoad() const = 0;
    virtual void incrementWorkLoad() = 0;
    virtual void decrementWorkLoad() = 0;

    virtual sycl::queue& checkoutQueue() = 0;
    virtual void checkinQueue() = 0;
    // Used to get queue when no lock is needed.
    virtual const sycl::queue& getQueue() const = 0;
};