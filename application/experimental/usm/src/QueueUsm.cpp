#include "EventUsm/QueueUsm.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "Logger/Logger.h"


const DeviceResourceGroup QueueUsm::NullResourceGroup = {};
const DeviceResourceGroup QueueUsm::NullResultResourceGroup = {};

QueueUsm::QueueUsm(sycl::queue& syclQueue, Capacity eventResourcesCapacity, Capacity resultResourcesCapacity, Capacity workCapacity)
: syclQueue_(syclQueue)
, eventResourcesCapacity_(eventResourcesCapacity)
, resultResourcesCapacity_(resultResourcesCapacity)
, workLoadCapacity_(workCapacity)
{
    // Allocate resources on startup to avoid overhead during event processing.
    for (DeviceResourceGroupId id = NullEventResourceGroupId + 1; id <= NullEventResourceGroupId + eventResourcesCapacity_; ++id)
    {
        eventResources_[id] = EventUsm::allocateDeviceResources(syclQueue_);
        freeEventResources_.push(id);
    }

    for (DeviceResourceGroupId id = NullResultResourceGroupId + 1; id <= NullResultResourceGroupId + resultResourcesCapacity_; ++id)
    {
        resultResources_[id] = ResultUsm::allocateDeviceResources(syclQueue_);
        freeResultResources_.push(id);
    }
}

QueueUsm::~QueueUsm()
{
    for (auto& resourceGroup : eventResources_)
    {
        EventUsm::deallocateDeviceResources(*resourceGroup.second, syclQueue_);
    }

    for (auto& resourceGroup : resultResources_)
    {
        ResultUsm::deallocateDeviceResources(*resourceGroup.second, syclQueue_);
    }
}

std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> QueueUsm::getEventResourceGroup()
{
    if (freeEventResources_.empty())
    {
        LOG_ERROR("No free event resources available in the queue.");
        return {NullEventResourceGroupId, NullResourceGroup};
    }

    DeviceResourceGroupId id = freeEventResources_.front();
    freeEventResources_.pop();
    return {id, *eventResources_[id]};
}

void QueueUsm::returnEventResourceGroup(DeviceResourceGroupId resourceGroupId)
{
    freeEventResources_.push(resourceGroupId);
}

std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> QueueUsm::getResultResourceGroup()
{
    if (freeResultResources_.empty())
    {
        LOG_ERROR("No free result resources available in the queue.");
        return {NullResultResourceGroupId, NullResultResourceGroup};
    }

    DeviceResourceGroupId id = freeResultResources_.front();
    freeResultResources_.pop();
    return {id, *resultResources_[id]};
}

void QueueUsm::returnResultResourceGroup(DeviceResourceGroupId resourceGroupId)
{
    freeResultResources_.push(resourceGroupId);
}

sycl::queue& QueueUsm::checkoutQueue()
{
    syclQueueMutex_.lock();
    return syclQueue_;
}

void QueueUsm::checkinQueue()
{
    syclQueueMutex_.unlock();
}
