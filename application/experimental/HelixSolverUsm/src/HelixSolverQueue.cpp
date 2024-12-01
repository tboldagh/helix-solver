#include "HelixSolverUsm/HelixSolverQueue.h"

HelixSolverQueue::HelixSolverQueue(sycl::queue& syclQueue, Capacity eventResourcesCapacity, Capacity resultResourcesCapacity, Capacity workCapacity, const Splitter& splitter)
: QueueUsm(syclQueue, eventResourcesCapacity, resultResourcesCapacity, workCapacity)
, splitter_(splitter)
{
    for (auto& resourceGroup : eventResources_)
    {
        resourceGroup.second->emplace(DeviceResourceType::Splitter, sycl::malloc_device<Splitter>(1, syclQueue_));
        syclQueue_.memcpy(resourceGroup.second->at(DeviceResourceType::Splitter), &splitter_, sizeof(Splitter)).wait();

    }
}

HelixSolverQueue::~HelixSolverQueue()
{
    for (auto& resourceGroup : eventResources_)
    {
        sycl::free(resourceGroup.second->at(DeviceResourceType::Splitter), syclQueue_);
    }
}