#pragma once

#include "EventUsm/DeviceResource.h"
#include "EventUsm/IQueue.h"

#include <queue>
#include <unordered_map>
#include <mutex>

class QueueUsm : public IQueue
{
public:
    QueueUsm(sycl::queue& syclQueue, Capacity eventResourcesCapacity, Capacity resultResourcesCapacity, Capacity workCapacity);
    ~QueueUsm() override;

    inline Capacity getEventResourcesCapacity() const override;
    inline Capacity getEventResourcesLoad() const override;
    std::pair<DeviceResourceGroupId, const DeviceResourceGroup&> getEventResourceGroup() override;
    void returnEventResourceGroup(DeviceResourceGroupId resourceGroupId) override;

    inline Capacity getResultResourcesCapacity() const override;
    inline Capacity getResultResourcesLoad() const override;
    std::pair<DeviceResourceGroupId, const DeviceResourceGroup&> getResultResourceGroup() override;
    void returnResultResourceGroup(DeviceResourceGroupId resourceGroupId) override;

    inline Capacity getWorkCapacity() const override;
    inline Capacity getWorkLoad() const override;
    inline void incrementWorkLoad() override;
    inline void decrementWorkLoad() override;

    sycl::queue& checkoutQueue() override;
    void checkinQueue() override;
    inline const sycl::queue& getQueue() const override;

    static const DeviceResourceGroup NullResourceGroup; // Used when no resources are available.
    static constexpr DeviceResourceGroupId NullEventResourceGroupId{0};

    static const DeviceResourceGroup NullResultResourceGroup; // Used when no resources are available.
    static constexpr DeviceResourceGroupId NullResultResourceGroupId{0};

protected:
    sycl::queue& syclQueue_;
    std::mutex syclQueueMutex_;

    const Capacity eventResourcesCapacity_;
    Capacity eventResourcesLoad_ = 0;
    std::unordered_map<DeviceResourceGroupId, std::unique_ptr<DeviceResourceGroup>> eventResources_;
    std::queue<DeviceResourceGroupId> freeEventResources_;

    const Capacity resultResourcesCapacity_;
    Capacity resultResourcesLoad_ = 0;
    std::unordered_map<DeviceResourceGroupId, std::unique_ptr<DeviceResourceGroup>> resultResources_;
    std::queue<DeviceResourceGroupId> freeResultResources_;

    const Capacity workLoadCapacity_;
    Capacity workLoad_ = 0;
};

inline IQueue::Capacity QueueUsm::getEventResourcesCapacity() const
{
    return eventResourcesCapacity_;
}

inline IQueue::Capacity QueueUsm::getEventResourcesLoad() const
{
    return eventResourcesLoad_;
}

inline IQueue::Capacity QueueUsm::getResultResourcesCapacity() const
{
    return resultResourcesCapacity_;
}

inline IQueue::Capacity QueueUsm::getResultResourcesLoad() const
{
    return resultResourcesLoad_;
}

inline IQueue::Capacity QueueUsm::getWorkCapacity() const
{
    return workLoadCapacity_;
}

inline IQueue::Capacity QueueUsm::getWorkLoad() const
{
    return workLoad_;
}

inline void QueueUsm::incrementWorkLoad()
{
    ++workLoad_;
}

inline void QueueUsm::decrementWorkLoad()
{
    --workLoad_;
}

inline const sycl::queue& QueueUsm::getQueue() const
{
    return syclQueue_;
}