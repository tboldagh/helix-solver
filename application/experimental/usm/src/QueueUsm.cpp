#include "EventUsm/QueueUsm.h"
#include "EventUsm/EventUsm.h"
#include "EventUsm/ResultUsm.h"
#include "Logger/Logger.h"


const DeviceResourceGroup QueueUsm::NullResourceGroup = {};

QueueUsm::QueueUsm(sycl::queue& syclQueue, Capacity resourcesCapacity, Capacity workCapacity)
: syclQueue_(syclQueue)
, resourcesCapacity_(resourcesCapacity)
, workLoadCapacity_(workCapacity)
{
}

QueueUsm::~QueueUsm()
{
    for (auto& [id, resourceGroup] : resources_)
    {
        if (resourceGroup)
        {
            for (auto& [type, resource] : *resourceGroup)
            {
                static_cast<KernelMemory*>(resource)->deallocate();
            }
        }
    }
}

bool QueueUsm::createResources(const CreateResourceGroupFunction& createResourceGroupFunction)
{
    for (DeviceResourceGroupId id = NullResourceGroupId + 1; id <= NullResourceGroupId + resourcesCapacity_; ++id)
    {
        auto resourceGroup = createResourceGroupFunction(syclQueue_);
        if (!resourceGroup)
        {
            LOG_ERROR("Failed to create resources, id: " + std::to_string(id));
            return false;
        }

        resources_[id] = std::move(resourceGroup);
        freeResources_.push(id);
    }

    return true;
}

std::pair<IQueue::DeviceResourceGroupId, const DeviceResourceGroup&> QueueUsm::getResources()
{
    if (freeResources_.empty())
    {
        LOG_ERROR("No free resources available in the queue.");
        return {NullResourceGroupId, NullResourceGroup};
    }

    DeviceResourceGroupId id = freeResources_.front();
    freeResources_.pop();
    resourcesLoad_++;
    return {id, *resources_[id]};
}

void QueueUsm::returnResources(DeviceResourceGroupId resourceGroupId)
{
    freeResources_.push(resourceGroupId);
    resourcesLoad_--;
}

void QueueUsm::forEachResourceGroup(const ForEachResourceGroupFunction& callback)
{
    for (auto& [id, resourceGroup] : resources_) {
        callback(id, *resourceGroup);
    }
}
