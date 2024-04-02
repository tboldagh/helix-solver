#pragma once

#include <map>
#include <memory>

class IQueue
{
public:
    using Capacity = u_int16_t;
    using ResourceType = u_int8_t;
    using Resources = std::map<ResourceType, void*>;

    virtual ~IQueue() = default;

    virtual Capacity getEventResourcesCapacity() const = 0;
    virtual Capacity getEventResourcesLoad() const = 0;
    virtual std::unique_ptr<Resources> releaseEventResources() const = 0;
    virtual void takeEventResources(std::unique_ptr<Resources> resources) = 0;

    virtual Capacity getResultResourcesCapacity() const = 0;
    virtual Capacity getResultResourcesLoad() const = 0;
    virtual std::unique_ptr<Resources> releaseResultResources() const = 0;
    virtual void takeResultResources(std::unique_ptr<Resources> resources) = 0;

    virtual Capacity getWorkCapacity() const = 0;
    virtual Capacity getWorkLoad() const = 0;
};